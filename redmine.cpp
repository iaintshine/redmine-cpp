#include <cpprest/http_client.h>
#include <cpprest/json.h>

#include <vector>
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
    static const utility::string_t projects_url;

    static const utility::string_t api_key;
    static const utility::string_t api_key_header;
};

const utility::string_t redmine_creds::base_url     = U("http://redmine.example.com");
const utility::string_t redmine_creds::users_url    = U("/users.json");
const utility::string_t redmine_creds::projects_url = U("/projects/");

const utility::string_t redmine_creds::api_key      = U("API_KEY");
const utility::string_t redmine_creds::api_key_header = U("X-Redmine-API-Key");

//
// redmine project
//
struct redmine_project
{
    int32_t             id;
    utility::string_t   name;
    utility::string_t   identifier;
};

//
// redmine user
//
struct redmine_user
{
    int32_t             id;
    utility::string_t   login;
    utility::string_t   first_name;
    utility::string_t   last_name;
};

//
// Redmine client
//
class redmine
{
public:
    //
    // query all users
    //
    static pplx::task<vector<redmine_user>> users(const uint32_t offset = 0UL, const uint32_t limit = 25UL);

    //
    // query project info
    //
    static pplx::task<redmine_project> project(utility::string_t const & name);
};

pplx::task<vector<redmine_user>> redmine::users(const uint32_t offset, const uint32_t limit)
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
                    vector<redmine_user> users_collection;

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

                        redmine_user user;
                            user.id         = id.as_integer();
                            user.login      = login.as_string();
                            user.first_name = first_name.as_string();
                            user.last_name  = last_name.as_string();

                        users_collection.push_back(std::move(user));
                    }

                    return std::move(users_collection);
                }
                catch (exception & e)
                {
                    ucerr << U("error occurred during data extraction: ") << e.what() << endl;
                }
            }

            return vector<redmine_user>();
        });
}

pplx::task<redmine_project> redmine::project(utility::string_t const & name)
{
    // define a http client that maintains the connection to a HTTP service
    http_client client(redmine_creds::base_url);

    // build the query parameters
    auto query = uri_builder(redmine_creds::projects_url)
            .append_path(name + U(".json"))
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
            return response.extract_json();
        })
        // continue when the son value is available
        .then([](json::value project_json)
        {
            if (!project_json.is_null())
            {
                try
                {
                    //
                    // Display all users
                    //
                    auto p          = project_json[U("project")];
                    auto id         = p[U("id")];
                    auto name       = p[U("name")];
                    auto identifier = p[U("identifier")];

                    redmine_project project;
                            project.id          = id.as_integer();
                            project.name        = name.as_string();
                            project.identifier  = identifier.as_string();

                    return std::move(project);
                }
                catch (exception & e)
                {
                    ucerr << U("error occurred during data extraction: ") << e.what() << endl;
                }
            }

            return redmine_project();
        });
}

#ifdef _MS_WINDOW
int wmain(int argc, wchar_t *args[])
#else
int main(int argc, char *args[])
#endif
{
    redmine::project("rest_test")
        .then([](redmine_project p)
        {
            ucout << p.id << "|" 
                  << p.name << "|"
                  << p.identifier
                  << endl;
        })
        .wait();

    

    return 0;
}