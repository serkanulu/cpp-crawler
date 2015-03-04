#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <boost/network/protocol/http/client.hpp>
#include <boost/network/uri.hpp>
#include <boost/network//uri/uri_io.hpp>
#include <htmlcxx/html/ParserDom.h>

using namespace std;
using namespace htmlcxx;
using namespace boost::network;

int main(int argc, char** argv)
{
   if (argc != 3) {
       std::cout << "Usage: " << argv[0] << " [url]" << " [depth]" << std::endl;
       return 1;
   }

   uri::uri url(argv[1]);
   cout << "Scheme: " << url.scheme() << endl;
   cout << "Host: " << url.host() << endl;
   cout << "Path: " << url.path() << endl;
   cout << "Fragment: " << url.fragment() << endl;

   std::string contents;
   http::client client;
   http::client::request request(argv[1]);
   request << header("Connection", "close");
   http::client::response response = client.get(request);

   contents = body(response);

   //Parse some html code
   HTML::ParserDom parser;
   tree<HTML::Node> dom = parser.parseTree(contents);

   //Dump all links in the tree
   for (auto it : dom)
   {
      if (strcasecmp(it.tagName().c_str(), "A") == 0)
      {
        it.parseAttributes();
        cout << it.attribute("href").second << endl;
      }
   }

   std::string pagetext;
   //Dump all text of the document
   for (auto it : dom)
   {
      if ((!it.isTag()) && (!it.isComment()))
      {
         pagetext += it.text();
      } 
   }
   cout << endl << endl;

   std::map<std::string, int> uniquewords;
   std::map<std::string, int>::iterator word_it;
   std::string result_text;
   std::string word;
   std::replace_copy_if(pagetext.begin(), pagetext.end(),            
                       std::back_inserter(result_text), //Store output           
                       std::ptr_fun<int, int>(&std::ispunct),
                       ' ');
   std::stringstream ss(result_text);
   while (ss >> word)
   {
      std::transform(word.begin(), word.end(), word.begin(), ::tolower);
      // word does not exist in map init count to 1
      if ( ( word_it = uniquewords.find(word) ) == uniquewords.end() )
      { 
         uniquewords[word] = 1;
      }
      // otherwise increment count
      else
      {
         uniquewords[word] = word_it->second + 1;
      }
   }

   for (auto word : uniquewords )
   {
      cout << word.first << " - count: " << word.second << endl;
   }

   return 0;
}
