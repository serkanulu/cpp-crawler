# cpp-crawler
Experimental C++ web crawler

This is a basic application that crawls a given url (http) n-links deep and calculates the 
number of unique words found on each page and in total.

Libraries used:

- boost_1_57_0
- cpp-netlib-0.11.1
- htmlcxx-0.85


Build using:

g++ crawler.cpp -o crawler -I /usr/local/include -L/usr/local/lib -lhtmlcxx -lcss_parser_pp -lcss_parser -lboost_system -lboost_filesystem -lboost_thread -lpthread -lcppnetlib-uri -lcppnetlib-client-connections -lssl -lcrypto -std=c++11
