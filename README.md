Twitter Streaming API test in command line
==========

depend
http://github.com/leico/libTwitterOAuth

usage
-----
1. clone and move repository

    ```
    git clone https://github.com/leico/commandline_userstream
    cd commandline_userstream
    ```
1. prepare library and headers

    ```
    ./preparelibrary.sh
    ```

1. write your keys in key.json

    ```
    {
        "consumer_key"    : "Your app consumer key"
      , "consumer_secret" : "Your app consumer secret"
      , "access_token"    : "Your access token"
      , "token_secret"    : "Your access token secret"
    }

    ```

1. build on XCode
