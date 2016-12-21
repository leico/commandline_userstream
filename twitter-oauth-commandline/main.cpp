//
//  main.cpp
//  twitter-oauth-commandline
//
//  Created by leico on 2016/12/19.
//  Copyright © 2016年 leico. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <sstream>
#include <map>
#include <ctime>
#include <algorithm>
#include <locale>
#include <oauth.h>
#include <curl/curl.h>
#include <picojson.h>

const std :: string gen_nonce(void);
const std :: string curl_tostring(char* cstr);
const std :: string replace(const std :: string str, const std :: string target, const std :: string repstr);

size_t write_callback   (char* data,      size_t size,    size_t nmemb, void  *userdata);
int    progress_callback(void* callcount, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

int main(int argc, const char * argv[]) {

  //check current working directory
  {
    char *cwd = getcwd(NULL, 0);
    std :: cout << "current working directory:" << cwd << std :: endl;
    free(cwd);
  }



  //OAuth parameters : URL and REST Method
  std :: string method = "GET";
  std :: string url    = "https://userstream.twitter.com/1.1/user.json";


  //get authentication key from file
  std :: ifstream ifs;
  ifs.open("key.json", std :: ios :: binary);
  if(ifs.fail()){
    std :: cout << "key.json open error" << std :: endl;
    exit(1);
  }

  picojson :: value json;
  ifs >> json;
  std :: string err = picojson :: get_last_error();
  if(! err.empty()){
    std :: cout << "key.json parse error" << std :: endl;
    exit(1);
  }

  picojson :: value :: object& keydata = json.get<picojson :: object>();

  std :: string consumer_key    = keydata["consumer_key"]   .get<std :: string>();
  std :: string consumer_secret = keydata["consumer_secret"].get<std :: string>();
  std :: string access_token    = keydata["access_token"]   .get<std :: string>();
  std :: string token_secret    = keydata["token_secret"]   .get<std :: string>();


  std :: cout << "consumer_key    : " << consumer_key    << "\n"
              << "consumer_secret : " << consumer_secret << "\n"
              << "access_token    : " << access_token    << "\n"
              << "token_secret    : " << token_secret    << std :: endl;


  //curl initialization
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();

  if(curl == NULL){
    std :: cerr << "curl is NULL" << std :: endl;
    exit(1);
  }



  //collect OAuth status
  using OAuthtype = std :: map<std :: string, std :: string>;
  OAuthtype OAuth_param;
  {
    std :: stringstream ss;
    ss << std :: time(nullptr);

    OAuth_param.insert( OAuthtype :: value_type("oauth_consumer_key", consumer_key.c_str()) );
    OAuth_param.insert( OAuthtype :: value_type("oauth_signature_method", "HMAC-SHA1")      );
    OAuth_param.insert( OAuthtype :: value_type("oauth_timestamp", ss.str().c_str())        );
    OAuth_param.insert( OAuthtype :: value_type("oauth_version", "1.0")                     );
    OAuth_param.insert( OAuthtype :: value_type("oauth_nonce", gen_nonce() .c_str())        );
    OAuth_param.insert( OAuthtype :: value_type("oauth_token", access_token.c_str())        );
  }


  //contruct paramter string
  std :: string query = [&](){

    std :: stringstream ss;

    for(OAuthtype :: iterator it = OAuth_param.begin() ; it != OAuth_param.end() ; ++ it){

      if(it != OAuth_param.begin()) ss << '&';

      ss        << curl_tostring( curl_easy_escape(curl, (it -> first) .c_str(), 0) );
      ss << '=' << curl_tostring( curl_easy_escape(curl, (it -> second).c_str(), 0) );
    }
    return ss.str();
  }();

  //contruct signature base string
  query = [&](){
    std :: stringstream ss;
    ss        << curl_tostring( curl_easy_escape(curl, method.c_str(), 0) );
    ss << '&' << curl_tostring( curl_easy_escape(curl, url   .c_str(), 0) );
    ss << '&' << curl_tostring( curl_easy_escape(curl, query .c_str(), 0) ); 

    return ss.str();
  }();


  //getting signing key string
  std :: string key = [&](){

    std :: stringstream ss;
    ss        << curl_tostring( curl_easy_escape(curl, consumer_secret.c_str(), 0) );
    ss << '&' << curl_tostring( curl_easy_escape(curl, token_secret   .c_str(), 0) );

    return ss.str();
  }();

  //calicurating signature and add signature parameter
  {
    std :: string signature = curl_tostring( oauth_sign_hmac_sha1(query.c_str(), key.c_str()) );
    signature = curl_tostring( curl_easy_escape    (curl, signature.c_str(), 0) );

    OAuth_param.insert( OAuthtype :: value_type("oauth_signature", signature) );
  }


  //creating Authorization header
  std :: string header = [&](){
    std :: stringstream ss;

    for(OAuthtype :: iterator it = OAuth_param.begin() ; it != OAuth_param.end() ; ++ it){

      ss << ( (it == OAuth_param.begin()) ? "Authorization: OAuth " : ", " );
      ss << (it -> first) << '=' << '\"' << (it -> second) << '\"';
    }

    return ss.str();
  }();

  //curl setting
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "testmax7");

  struct curl_slist *httpheader = NULL;
                     httpheader = curl_slist_append(httpheader, header.c_str() );

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, httpheader);
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");




  std :: string receive_data(65535, '\0');
                receive_data.clear();

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA,     reinterpret_cast<void *>(&receive_data));



  int progresscall_count = 0;

  curl_easy_setopt(curl, CURLOPT_NOPROGRESS,       0L);
  curl_easy_setopt(curl, CURLOPT_XFERINFODATA,     reinterpret_cast<void *>(&progresscall_count) );
  curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);


  //connect twitter
  CURLcode curlstatus = curl_easy_perform(curl);

  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

  //output log
  std :: cout << http_code << ":" << curl_easy_strerror( curlstatus ) << std :: endl;

  //cleanup
  curl_slist_free_all(httpheader);
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return 0;
}





//generate 32 byte random string and base 64 encode
const std :: string gen_nonce(void){

  //twitter need 32 byte data
  const int LETTER_COUNT = 32;

  std :: stringstream ss;

  //generate more than 32 letters string
  while(ss.tellp() <= LETTER_COUNT)
    ss << curl_tostring( oauth_gen_nonce() );

  //trim 32 letters
  std :: string str = ss.str().substr(0, LETTER_COUNT);

  //and, need base 64 encording 
  str = curl_tostring( 
          oauth_encode_base64(
              LETTER_COUNT
            , reinterpret_cast<const unsigned char *>( str.c_str() ) 
          )
        );

  //also need to remove non word(non A-Z, a-z, 0-9) charactors
  str.erase(
    std :: remove_if(
        str.begin()
      , str.end()
      , [](char c) -> bool { return (std :: isalnum(c) == 0); }
    )
    , str.end()
  );

  return str;
}





//copy string returned and free curl function char*
const std :: string curl_tostring(char* cstr){
  std :: string str = cstr;
  curl_free(cstr);    //curl function returned char*, it need to free
  return str;
}




//replace part of stging 
const std :: string replace(const std :: string str, const std :: string target, const std :: string repstr){
  std :: string data = str;
  std :: string :: size_type pos( data.find(target) );

  while(pos != std :: string :: npos){

    data.replace(pos, target.size(), repstr);

    pos = data.find(target, pos + repstr.size());

  }

  return data;
}

//called when data get
size_t write_callback(char* strdata, size_t size, size_t nmemb, void *userdata){
  
  size_t realsize = size * nmemb;


  const std :: string ENDMARK("\r\n");

  std :: string *receive_data = reinterpret_cast<std :: string *>(userdata);
  std :: string  str(strdata, realsize);

  if(str == ENDMARK)      //end when no data
    return realsize;

  //conbine download data
  (*receive_data) += str;
  
  //end when data splitting
  if(receive_data -> find(ENDMARK) == std :: string :: npos) 
    return realsize;

  //combine all data of a tweet

  std :: cout << "size    :" << receive_data -> size()  << std :: endl;
  std :: cout << "Get Data:"                            << std :: endl;
  std :: cout << *receive_data                          << std :: endl;
  std :: cout                                           << std :: endl;




  picojson :: value json;
  std      :: string err("");

  picojson :: parse(json, receive_data -> begin(), receive_data -> end(), &err);


  //end when json parse error
  if(! err.empty()){
    std :: cout << "json error:" << std :: endl;
    receive_data -> clear();
    return realsize;
  }

  //json decode(as well parse unicode escape sequence) and parse escape sequence
  std :: string data = json.serialize();
  data = replace(data, "\\/", "/");
  data = replace(data, "\\\"", "\"");
  data = replace(data, "\\n", "\n");
  std :: cout << "json serialize:" << std :: endl;
  std :: cout << data              << std :: endl;

  receive_data -> clear();


  return realsize;
}




//called when perform
int progress_callback(void* callcount, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow){

  int &count = *(reinterpret_cast<int *>(callcount));

  count ++;
  
  std :: cout << "DLtotal : " << dltotal << '\n'
              << "DLnow   : " << dlnow   << '\n'
              << "ULtotal : " << ultotal << '\n'
              << "ULnow   : " << ulnow   << '\n'
              << "count   : " << count   << std :: endl;

  if(count > 100) return 1; //if return non zero, perform stopped

  return 0;
}

