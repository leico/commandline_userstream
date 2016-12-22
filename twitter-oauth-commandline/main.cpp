//
//  main.cpp
//  twitter-oauth-commandline
//
//  Created by leico on 2016/12/19.
//  Copyright © 2016年 leico. All rights reserved.
//

#include <iostream>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include <picojson.h>
#include <curl/curl.h>
#include <oauth.h>
#include "TwitterOAuth.hpp"

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

  std :: string receive_data(65535, '\0');
                receive_data.clear();

  int progresscall_count = 0;

  TwitterOAuth twitter(
      method
    , url
    , consumer_key
    , consumer_secret
    , access_token
    , token_secret
    , true
    , "testmax7"
    , write_callback
    , progress_callback
    , reinterpret_cast<void *>(&receive_data)
    , reinterpret_cast<void *>(&progresscall_count)
  );

  
  std :: cout << twitter.SendRequest() << std :: endl;

  return 0;
  
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

