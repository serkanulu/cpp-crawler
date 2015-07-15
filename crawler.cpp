#include <string>
#include <algorithm>
#include <iterator>
#include <list>
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


std::string get_url_contents(std::string url)
{
    std::string contents;
    http::client client;
    http::client::request request(url);
    request << header("Connection", "close");
    http::client::response response = client.get(request);

    contents = body(response);

    return contents;
}

void parse_webpage(std::string url, std::string pagecode, std::list<std::string> &links, std::string &pagetext)
{
    //Parse some html code
    HTML::ParserDom parser;
    tree<HTML::Node> dom = parser.parseTree(pagecode);

    //Dump all links in the tree
    for (auto it : dom)
    {
        if (strcasecmp(it.tagName().c_str(), "A") == 0)
        {
            it.parseAttributes();
            std::string link = it.attribute("href").second;

            cout << endl;

            uri::uri linkurl(link);

            // store if absolute url
            if ( linkurl.scheme() == "http" )
            {
                links.push_back(link);
            }
            // otherwise create absolute link from relative href
            else if ( !uri::is_hierarchical(linkurl) && !uri::is_opaque(linkurl) && !link.empty() )
            {
                uri::uri baseurl(url);

                std::string newlink = baseurl.scheme() + "://" + baseurl.host();

                // handle links relative from the same path
                if ( isalpha(link.at(0)) || link.at(0) == '.' )
                {
                    std::string newpath = ( !baseurl.path().empty() ) ? baseurl.path() : "/";
                    newlink += newpath;
                    newlink.replace(newlink.rfind("/")+1, std::string::npos, link);
                    links.push_back(newlink);
                }
                // handle links relative from the base url/host
                else if ( link.at(0) == '/' )
                {
                    newlink += link;
                    links.push_back(newlink);
                }

            }
        }
    }

    if ( !links.empty() )
    {
        links.sort();
        links.unique();

        cout << endl;
        for (auto it : links)
            cout << it << endl;
    }

    //Dump all text of the document
    tree<HTML::Node>::iterator it = dom.begin();
    tree<HTML::Node>::iterator end = dom.end();
    for (; it != end; ++it)
    {
        if ( (!it->isTag()) && (!it->isComment()) )
        {
            pagetext += it->text();
        } 
        // htmlcxx doesn't check for script and style tags by default
        // so skip any text that is pure javascript and css
        else if ( it->tagName() == "script" || it->tagName() == "style" )
        {
            it.skip_children();
        }
    }
}


void add_word(std::map<std::string, int> &word_map, std::string word)
{
    std::map<std::string, int>::iterator word_it;

    // words forced to lower case to avoid duplicates with different case
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
    // word does not exist in map init count to 1
    if ( ( word_it = word_map.find(word) ) == word_map.end() )
    { 
        word_map[word] = 1;
    }
    // otherwise increment count
    else
    {
        word_map[word] = word_it->second + 1;
    }
}

void find_unique_words(std::string url, int depth, std::map<std::string, std::map<std::string, int>> &sitemap,
                       std::map<std::string, int> &totals)
{
    std::list<std::string> links;
    std::string pagetext;

    if ( depth >= 0 )
    {
        std::map<std::string, int> uniquewords;
        std::map<std::string, int>::iterator word_it;
        std::string result_text;
        std::string word;

        std::string page_content = get_url_contents(url);
        parse_webpage(url, page_content, links, pagetext);

        // handle next level of links
        for (auto link : links)
        {
            find_unique_words(link, depth - 1, sitemap, totals);
        }

        // replace all punctuation with space
        std::replace_copy_if(pagetext.begin(), pagetext.end(),            
                std::back_inserter(result_text), //Store output           
                std::ptr_fun<int, int>(&std::ispunct),
                ' ');

        // split web text on space
        std::stringstream ss(result_text);
        while (ss >> word)
        {
            // add word to current link's/page's word map
            add_word(uniquewords, word);
            // also increment the count in the grand total
            add_word(totals, word);
        }

        // set word map for this page
        sitemap[url] = uniquewords;
    }
}


int main(int argc, char** argv)
{
    std::map<std::string, std::map<std::string, int>> sitemap;
    std::map<std::string, int> totals;

    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " [url]" << " [depth]" << std::endl;
        return 1;
    }

    // create a URI from the url argument and validate it
    std::string urlstr = argv[1];
    int depth = atoi(argv[2]);

    cout << "depth: " << depth << endl;
    if ( depth < 0 )
    {
        std::cout << "Invalid depth [" << depth << "], must be 0 or greater!" << endl;
    }

    uri::uri url(urlstr);
    if ( !url.is_valid() )
    {
        std::cout << "Invalid URL: " << urlstr << endl;
        return 1;
    }

    /*
       cout << "Scheme: " << url.scheme() << endl;
       cout << "Host: " << url.host() << endl;
       cout << "Path: " << url.path() << endl;
       cout << "Fragment: " << url.fragment() << endl;
       */

    find_unique_words(urlstr, depth, sitemap, totals);

    for ( auto &link : sitemap )
    {    
        cout << "Processing URL: " << link.first << endl << endl;
        for ( auto &word : link.second )
        {
            cout << word.first << " - count: " << word.second << endl;
        }
        cout << endl;
    }

    cout << endl;
    cout << "Unique Words in all pages: " << endl << endl;
    for ( auto &word : totals )
    {
        cout << word.first << " - count: " << word.second << endl;
    }
    cout << endl;

    return 0;
}
