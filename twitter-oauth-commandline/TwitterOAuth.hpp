//
//  TwitterOAuth.hpp
//  twitter-oauth-commandline
//
//  Created by leico on 2016/12/22.
//  Copyright © 2016年 leico. All rights reserved.
//

#ifndef __TwitterOAuth_hpp___
#define __TwitterOAuth_hpp___

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <locale>
#include <oauth.h>
#include <curl/curl.h>
#include <picojson.h>

#endif /* TwitterOAuth_hpp */

class TwitterOAuth{
  
  using StrMap  = std :: map<std :: string, std :: string>;
  using ustring = std :: basic_string<unsigned char>;


  private:

  static const int NONCE_LETTER_COUNT  = 32;

  static const std :: string SIGNATURE_METHOD;
  static const std :: string OAUTH_VERSION;

  static size_t (*const NULL_RESPONSEFUNC)(char*, size_t, size_t, void*);
  static int    (*const NULL_PROGRESSFUNC)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t);

  StrMap _oauthdata;
  StrMap _restdata;
  StrMap _signaturedata;

  CURL *_curl;

  bool          _curl_verbose;
  std :: string _curl_agent;

  size_t (*_responsefunc)(char*, size_t, size_t, void*);
  int    (*_progressfunc)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t);
  void *_data_responsecallback;
  void *_data_progresscallback;



  public:

           TwitterOAuth(void);
  virtual ~TwitterOAuth(void);





  TwitterOAuth( const std :: string& rest_method
              , const std :: string& url
              , const std :: string& consumer_key
              , const std :: string& consumer_secret
              , const std :: string& access_token
              , const std :: string& token_secret

              , const bool           curl_verbose   = false
              , const std :: string& curl_useragetn = ""

              , size_t(*const response_callback)(char*, size_t, size_t, void*) = NULL_RESPONSEFUNC
              , int   (*const progress_callback)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t) = NULL_PROGRESSFUNC
              , void *const data_passedresponsecallback = NULL
              , void *const data_passedprogresscallback = NULL
  );

  const std :: string SendRequest(void);

      const std :: string GenerateQuery(void);
      const std :: string ConstructBaseString(const std :: string& query);
      const std :: string ConstructKeyString (void);
      const std :: string ConstructSignature(const std :: string& query, const std :: string& key);
      const std :: string ConstructAuthorizationHeader(void);

          const std :: string GenerateNonce        (const unsigned int lettercount);
          const ustring       GenerateRandomLetters(const unsigned int lettercount);
          const std :: string EncodeBase64         (const ustring&     letters);

      const bool CurlVerbose(const bool verbose);
      const bool CurlVerbose(void);

      const std :: string CurlUserAgent(const std :: string& agent);
      const std :: string CurlUserAgent(void);

  
  const std :: string& RESTMethod(void) const;
  const std :: string& RESTMethod(const std :: string& method);
  
  const std :: string& URL(void) const;
  const std :: string& URL(const std :: string& url);
  
  const std :: string& ConsumerKey(void) const;
  const std :: string& ConsumerKey(const std :: string& consumer_key);
  
  const std :: string& ConsumerSecret(void) const;
  const std :: string& ConsumerSecret(const std :: string& consumer_secret);
  
  const std :: string& AccessToken(void) const;
  const std :: string& AccessToken(const std :: string& access_token);
  
  const std :: string& AccessTokenSecret(void) const;
  const std :: string& AccessTokenSecret(const std :: string& token_secret);

  const bool isResponseCallback(void) const;
  const bool   ResponseCallback(size_t (*const response_callback)(char*, size_t, size_t, void*) );

  const bool isProgressCallback(void) const;
  const bool   ProgressCallback(int (*const progress_callback)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t) );

  const void * const PassedResponseCallbackData(void) const;
  const void * const PassedResponseCallbackData(void * const data);
  
  const void * const PassedProgressCallbackData(void) const;
  const void * const PassedProgressCallbackData(void * const data);


  private:

  void InitParam(void);
  const std :: string CurlToString(char* cstr);
};


inline void TwitterOAuth :: InitParam(void){

  using Pair = StrMap :: value_type;
  
  _oauthdata.insert( Pair("oauth_signature_method", SIGNATURE_METHOD) );
  _oauthdata.insert( Pair("oauth_version"         , OAUTH_VERSION) );
  _oauthdata.insert( Pair("oauth_consumer_key"    , "") );
  _oauthdata.insert( Pair("oauth_timestamp"       , "") );
  _oauthdata.insert( Pair("oauth_nonce"           , "") );
  _oauthdata.insert( Pair("oauth_token"           , "") );

  _restdata.insert( Pair("RESTmethod", "") );
  _restdata.insert( Pair("url"       , "") );

  _signaturedata.insert( Pair("consumer_secret", "") );
  _signaturedata.insert( Pair("token_secret", "") );



  curl_global_init(CURL_GLOBAL_ALL);
  _curl = curl_easy_init();

  if(_curl == NULL){
    std :: cout << "curl is NULL" << std :: endl;
    exit(1);
  }

  CurlVerbose  (false);
  CurlUserAgent("");

  return;
}






inline TwitterOAuth ::  TwitterOAuth(void){
  InitParam();
}

inline TwitterOAuth :: ~TwitterOAuth(void){

  curl_easy_cleanup(_curl);
  curl_global_cleanup();
}

inline TwitterOAuth :: TwitterOAuth( const std :: string& rest_method
                                   , const std :: string& url
                                   , const std :: string& consumer_key
                                   , const std :: string& consumer_secret
                                   , const std :: string& access_token
                                   , const std :: string& token_secret

                                   , const bool           curl_verbose
                                   , const std :: string& curl_useragent

                                   , size_t(*const response_callback)(char*, size_t, size_t, void*)
                                   , int   (*const progress_callback)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t)
                                   , void *const data_passedresponsecallback
                                   , void *const data_passedprogresscallback
                                   )
{
  InitParam();

  RESTMethod(rest_method);
  URL       (url);
 
  ConsumerKey(consumer_key);
  AccessToken(access_token);

  ConsumerSecret   (consumer_secret);
  AccessTokenSecret(token_secret);

  ResponseCallback(response_callback);
  ProgressCallback(progress_callback);

  PassedResponseCallbackData(data_passedresponsecallback);
  PassedProgressCallbackData(data_passedprogresscallback);
};

inline const std :: string TwitterOAuth :: GenerateNonce(const unsigned int lettercount){
  return EncodeBase64( GenerateRandomLetters(lettercount) );
}





inline const std :: basic_string<unsigned char> TwitterOAuth :: GenerateRandomLetters(const unsigned int lettercount){
  
  std :: basic_string<unsigned char> str;
  if(lettercount <= 0) return str;

  str.resize(lettercount);

  srand(static_cast<unsigned>(time(NULL)));

  for(int i = 0 ; i < lettercount ; ++ i)
    str.at(i) = rand() % UCHAR_MAX;

  return str;
}







inline const std :: string TwitterOAuth :: EncodeBase64(const std :: basic_string<unsigned char>& letters){
  return CurlToString( oauth_encode_base64(static_cast<int>(letters.size()), letters.data()) );
}









inline const std :: string& TwitterOAuth :: RESTMethod(void) const { return _restdata.at("RESTmethod"); }
inline const std :: string& TwitterOAuth :: URL       (void) const { return _restdata.at("url");        }

inline const std :: string& TwitterOAuth :: ConsumerKey(void) const { return _oauthdata.at("oauth_consumer_key"); }
inline const std :: string& TwitterOAuth :: AccessToken(void) const { return _oauthdata.at("oauth_token");        }

inline const std :: string& TwitterOAuth :: ConsumerSecret   (void) const { return _signaturedata.at("consumer_secret"); }
inline const std :: string& TwitterOAuth :: AccessTokenSecret(void) const { return _signaturedata.at("token_secret");    }

inline const bool TwitterOAuth :: isResponseCallback(void) const { return _responsefunc != NULL_RESPONSEFUNC;   }
inline const bool TwitterOAuth :: isProgressCallback(void) const { return _progressfunc != NULL_PROGRESSFUNC;   }

inline const void * const TwitterOAuth :: PassedResponseCallbackData(void) const { return _data_responsecallback; }
inline const void * const TwitterOAuth :: PassedProgressCallbackData(void) const { return _data_progresscallback; }


inline const std :: string& TwitterOAuth :: RESTMethod(const std :: string& method){ return _restdata.at("RESTmethod") = method; };
inline const std :: string& TwitterOAuth :: URL       (const std :: string& url)   { return _restdata.at("url")        = url;    };

inline const std :: string& TwitterOAuth :: ConsumerKey(const std :: string& consumer_key){ return _oauthdata.at("oauth_consumer_key") = consumer_key; }
inline const std :: string& TwitterOAuth :: AccessToken(const std :: string& access_token){ return _oauthdata.at("oauth_token")        = access_token; }

inline const std :: string& TwitterOAuth :: ConsumerSecret   (const std :: string& consumer_secret){ return _signaturedata.at("consumer_secret") = consumer_secret; }
inline const std :: string& TwitterOAuth :: AccessTokenSecret(const std :: string& token_secret)   { return _signaturedata.at("token_secret")    = token_secret;    }

inline const bool TwitterOAuth :: ResponseCallback(size_t (*const response_callback)(char*, size_t, size_t, void*) ){
  _responsefunc = response_callback;
  return true;
}

inline const bool TwitterOAuth :: ProgressCallback(int (*const progress_callback)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t) ){
  _progressfunc = progress_callback;
  return true;
}

inline const void * const TwitterOAuth :: PassedResponseCallbackData( void * const data){ return _data_responsecallback = data; }
inline const void * const TwitterOAuth :: PassedProgressCallbackData( void * const data){ return _data_progresscallback = data; }

inline const std :: string TwitterOAuth :: CurlToString(char* cstr){
  std :: string str = cstr;
  curl_free(cstr);
  return str;
}


inline const std :: string TwitterOAuth :: SendRequest(void){

  using Pair = StrMap :: value_type;

  std :: string query = ConstructBaseString( GenerateQuery() );
  std :: string key   = ConstructKeyString ();

  std :: string signature = ConstructSignature(query, key);
  
  _oauthdata.insert( Pair("oauth_signature", signature) );
  
  std :: string authorizationheader = ConstructAuthorizationHeader();

  //curl setting
  curl_easy_setopt(_curl, CURLOPT_URL, _restdata.at("url").c_str() );

  if(CurlVerbose())
    curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1L);

  if( ! CurlUserAgent().empty() )
    curl_easy_setopt(_curl, CURLOPT_USERAGENT, CurlUserAgent().c_str() );

  struct curl_slist *httpheader = NULL;
                     httpheader = curl_slist_append(httpheader, authorizationheader.c_str() );

  curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, httpheader);
  curl_easy_setopt(_curl, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(_curl, CURLOPT_ENCODING, "gzip");


  if(_responsefunc != NULL_RESPONSEFUNC)
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, _responsefunc);

  if(_data_responsecallback != NULL)
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, _data_responsecallback);

  if(_progressfunc != NULL_PROGRESSFUNC){
    curl_easy_setopt(_curl, CURLOPT_NOPROGRESS,       0L);
    curl_easy_setopt(_curl, CURLOPT_XFERINFOFUNCTION, _progressfunc);
  }

  if(_data_progresscallback != NULL)
    curl_easy_setopt(_curl, CURLOPT_XFERINFODATA, _data_progresscallback );

  //connect twitter
  CURLcode curlstatus = curl_easy_perform(_curl);

  //cleanup
  curl_slist_free_all(httpheader);


  long http_code = 0;
  curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &http_code);

  std :: stringstream ss;

  //output log
  ss << http_code << ":" << curl_easy_strerror( curlstatus );

  curl_easy_reset(_curl);

  return ss.str();

}


inline const std :: string TwitterOAuth :: GenerateQuery(void){

  _oauthdata.at("oauth_timestamp") = [&](){
    std :: stringstream ss;
    ss << std :: time(NULL);
    return ss.str();
  }();

  _oauthdata.at("oauth_nonce") = [&](){
    std :: string nonce = GenerateNonce(NONCE_LETTER_COUNT);
    
    nonce.erase(
      std :: remove_if(
          nonce.begin()
        , nonce.end  ()
        , [](char c) -> bool { return (std :: isalnum(c) == 0); }
      )
      , nonce.end()
    );

    return nonce;
  }();

  std :: string query = [&](){
    std :: stringstream ss;
    
    for(StrMap :: iterator it = _oauthdata.begin() ; it != _oauthdata.end() ; ++ it){
      
      if(it != _oauthdata.begin()) ss << '&';
     
      ss        << CurlToString( curl_easy_escape(_curl, (it -> first) .c_str(), 0) );
      ss << '=' << CurlToString( curl_easy_escape(_curl, (it -> second).c_str(), 0) );
    }

    return ss.str();
  }();


  return query;


}

inline const std :: string TwitterOAuth :: ConstructBaseString(const std :: string& query){
  std :: stringstream ss;
  ss        << CurlToString( curl_easy_escape(_curl, _restdata.at("RESTmethod").c_str(), 0) );
  ss << '&' << CurlToString( curl_easy_escape(_curl, _restdata.at("url")       .c_str(), 0) );
  ss << '&' << CurlToString( curl_easy_escape(_curl, query                     .c_str(), 0) ); 

  return ss.str();
}

inline const std :: string TwitterOAuth :: ConstructKeyString (void){

    std :: stringstream ss;
    ss        << CurlToString( curl_easy_escape(_curl, _signaturedata.at("consumer_secret").c_str(), 0) );
    ss << '&' << CurlToString( curl_easy_escape(_curl, _signaturedata.at("token_secret")   .c_str(), 0) );

    return ss.str();

}

inline const std :: string TwitterOAuth :: ConstructSignature(const std :: string& query, const std :: string& key){

    std :: string signature = CurlToString( oauth_sign_hmac_sha1(query.c_str(), key.c_str()) );
                  signature = CurlToString( curl_easy_escape    (_curl, signature.c_str(), 0) );

    return signature;
}

inline const std :: string TwitterOAuth :: ConstructAuthorizationHeader(void){
    std :: stringstream ss;

    for(StrMap :: iterator it = _oauthdata.begin() ; it != _oauthdata.end() ; ++ it){

      ss << ( (it == _oauthdata.begin()) ? "Authorization: OAuth " : ", " );
      ss << (it -> first) << '=' << '\"' << (it -> second) << '\"';
    }

    return ss.str();

}


inline const bool TwitterOAuth :: CurlVerbose(const bool verbose){ return _curl_verbose = verbose; }
inline const bool TwitterOAuth :: CurlVerbose(void)              { return _curl_verbose; }

inline const std :: string TwitterOAuth :: CurlUserAgent(const std :: string& agent){ return _curl_agent = agent; }
inline const std :: string TwitterOAuth :: CurlUserAgent(void)                      { return _curl_agent; }

