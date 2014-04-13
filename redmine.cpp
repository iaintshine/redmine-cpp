#include <cpprest/http_client.h>
#include <cpprest/json.h>

#include <iostream>
#include <stdexcept>

using namespace web;
using namespace web::http;
using namespace web::http::client; 

using namespace std;

//
// Credentials class to managet the service URLs and api-keys
//
class redmine_creds
{
public:
    static const utility::string_t base_url;
    static const utility::string_t users_url;

    static const utility::string_t api_key;
    static const utility::string_t api_key_header;
};

const utility::string_t redmine_creds::base_url     = U("http://redmine.realitypump.com");
const utility::string_t redmine_creds::users_url    = U("/users.json");
const utility::string_t redmine_creds::api_key      = U("9a7c782d37c9c1abf3e86bdd668f26769ead54dd");
const utility::string_t redmine_creds::api_key_header = U("X-Redmine-API-Key");

//
// Redmine client
//
class redmine
{
public:
    //
    // query all users
    //
    static pplx::task<void> users(const uint32_t offset = 0UL, const uint32_t limit = 25UL);
};

pplx::task<void> redmine::users(const uint32_t offset, const uint32_t limit)
{
    // validation
    if (limit > 100UL)
    {
        throw out_of_range("limit must be less or equal to 100");
    }

    // define a http client that maintains the connection to a HTTP service
    http_client client(redmine_creds::base_url);

    // build the query parameters
    auto query = uri_builder(redmine_creds::users_url)
            .append_query(U("offset"), offset)
            .append_query(U("limit"), limit)
            .to_string();

    http_request request(methods::GET);
        request.set_request_uri(query);
        request.headers().add(header_names::accept, U("application/json"));
        request.headers().add(redmine_creds::api_key_header, redmine_creds::api_key);

    return client
        // send the HTTP request asynchronous 
        .request(request)
        // continue when the response is available
        .then([](http_response response)
        {
#ifndef NDEBUG
           
            //
            // Debug show response headers 
            //
            for (auto h : response.headers())
            {
                ucout << h.first << " : " << h.second << endl;
            }

            //
            // Debug show response body, it adavences internal stream pointer so it won't be able to extract values twice
            //
            // auto body = response.extract_string().get();
            // ucout << body << endl;
#endif

            return response.extract_json();
        })
        // continue when the son value is available
        .then([](json::value users_json)
        {
            if (!users_json.is_null())
            {
                try
                {
                    //
                    // Display all users
                    //
                    auto users = users_json[U("users")];
                    for (auto& u : users.as_array())
                    {
                        auto id         = u[U("id")];
                        auto login      = u[U("login")];
                        auto first_name = u[U("firstname")];
                        auto last_name  = u[U("lastname")];

                        ucout << id.as_integer() << "|" 
                              << login.as_string() << "|"
                              << first_name.as_string() << "|"
                              << last_name.as_string()
                              << endl;
                    }
                }
                catch (exception & e)
                {
                    ucerr << U("error occurred during data extraction: ") << e.what() << endl;
                }
            }
        });
}

#ifdef _MS_WINDOW
int wmain(int argc, wchar_t *args[])
#else
int main(int argc, char *args[])
#endif
{
    redmine::users().wait();

    return 0;
}