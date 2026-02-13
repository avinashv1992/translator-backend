#include "crow/app.h"
#include "crow/json.h"
//#include "translator.h"


// int main(){

//     crow::SimpleApp app;

//     Translator translator;

//     // Health
//     CROW_ROUTE(app,"/health")
//     ([](){
//         return "OK";
//     });

//     // Translate API
//     CROW_ROUTE(app,"/api/translate")
//       .methods("POST"_method)
//     ([&](const crow::request& req){

//         auto body =
//           crow::json::load(req.body);

//         if(!body){
//             return crow::response(
//               400,"Invalid JSON");
//         }

//         std::string text =
//           body["text"].s();

//         std::string lang =
//           body["language"].s();

//         std::string result =
//           translator.translate(text,lang);

//         crow::json::wvalue resp;

//         resp["result"] = result;
//         resp["status"] = "ok";

//         return crow::response(resp);
//     });

//     app.port(8080)
//        .multithreaded()
//        .run();
// }

// Perfect. Below is a complete, single-file main.cpp that:

// ✅ Runs a Crow HTTP server
// ✅ Accepts JSON requests
// ✅ Builds a translation context
// ✅ Calls Groq/LLaMA using libcurl
// ✅ Handles errors safely
// ✅ Uses environment variable for API key
// ✅ Returns clean JSON to frontend

// This is a self-contained demo backend.

// You can copy–paste this directly.

// ✅ 1. Complete main.cpp (All-in-One Backend)

// Create / replace:

// backend/src/main.cpp


// with this:

// 📄 main.cpp
//#include "crow.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include <iostream>
#include <string>
#include <cstdlib>

// for caching translations (optional)
#include <unordered_map>
#include <mutex>
// Translation Cache (Thread-Safe)
std::unordered_map<std::string, std::string> translationCache;
std::mutex cacheMutex;


using json = nlohmann::json;

/* ================================
   Global Config
================================ */

// Read API key from environment
std::string getApiKey()
{
     const char* key = std::getenv("GROQ_API_KEY");

    if (!key)
    {
        throw std::runtime_error(
            "GROQ_API_KEY environment variable not set"
        );
    }

    return std::string(key);
}

const std::string GROQ_URL =
    "https://api.groq.com/openai/v1/chat/completions";

std::string GROQ_API_KEY;


/* ================================
   Curl Helper
================================ */

static size_t WriteCallback(
    void* contents,
    size_t size,
    size_t nmemb,
    std::string* output)
{
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}


/* ================================
   Context Builder
================================ */

std::string buildContext(
    const std::string& text,
    const std::string& language)
{
    // std::string context =
    //     "You are an enterprise localization engine.\n"
    //     "Rules:\n"
    //     "- Always return only translated text\n"
    //     "- No explanation\n"
    //     "- No extra symbols\n"
    //     "- Use formal UI language\n"
    //     "- Be consistent\n\n"
    //     "Translate to " + language + ":\n\"" +
    //     text + "\"";

    // return context;
    std::string context =
        "You are a professional enterprise software localization engine.\n"
        "You translate UI strings for Siemens Teamcenter PLM.\n"
        "Rules:\n"
        "1. Always preserve technical meaning.\n"
        "2. Always use standard PLM terminology.\n"
        "3. Never paraphrase.\n"
        "4. Never simplify.\n"
        "5. Never add explanation.\n"
        "6. Never change sentence structure.\n"
        "7. Always be consistent across requests.\n"
        "8. Output ONLY translated text.\n\n"
        "Target language: " + language + "\n\n"
        "Translate this UI string exactly:\n" + text + "";

      return context;

    // return R"(You are a professional enterprise software localization engine.

    // You translate UI strings for Siemens Teamcenter PLM.

    // Rules:
    // 1. Always preserve technical meaning.
    // 2. Always use standard PLM terminology.
    // 3. Never paraphrase.
    // 4. Never simplify.
    // 5. Never add explanation.
    // 6. Never change sentence structure.
    // 7. Always be consistent across requests.
    // 8. Output ONLY translated text.

    // Target language: )" + language + R"(

    // Translate this UI string exactly:

    // ")" + text + R"(")";
}


/* ================================
   LLM Client (Groq / LLaMA)
================================ */

std::string callGroqLLM(const std::string& prompt)
{
    CURL* curl = curl_easy_init();

    if (!curl)
        throw std::runtime_error("Curl init failed");

    std::string response;

    /* Build JSON body */
    json body = {
        {"model", "llama-3.1-8b-instant"},
        {"temperature", 0.0},
        {"messages", {
            {
                {"role", "system"},
                {"content", "You are a professional translator."}
            },
            {
                {"role", "user"},
                {"content", prompt}
            }
        }}
    };

    std::string bodyStr = body.dump();

    /* Headers */
    struct curl_slist* headers = nullptr;

    headers = curl_slist_append(
        headers, "Content-Type: application/json");

    std::string auth =
        "Authorization: Bearer " + GROQ_API_KEY;

    headers = curl_slist_append(
        headers, auth.c_str());

    /* Curl options */
    curl_easy_setopt(curl, CURLOPT_URL,
                     GROQ_URL.c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,
                     headers);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,
                     bodyStr.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     WriteCallback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                     &response);

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    /* Execute */
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        throw std::runtime_error(
            curl_easy_strerror(res));
    }

    /* Cleanup */
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    /* Parse response */
    auto j = json::parse(response);

    if (!j.contains("choices"))
        throw std::runtime_error("Invalid LLM response");

    return j["choices"][0]["message"]["content"];
}


/* ================================
   Translator Service
================================ */

std::string translateText(
    const std::string& text,
    const std::string& language)
{
    std::string context =
        buildContext(text, language);

    return callGroqLLM(context);
}

/* ================================
   saveToCache - Store translation in cache
================================ */
void saveToCache(
    const std::string& key,
    const std::string& value)
{
    std::lock_guard<std::mutex> lock(cacheMutex);

    translationCache[key] = value;
}

/* ================================
   getFromCache - Check if translation exists in cache
================================ */
bool getFromCache(
    const std::string& key,
    std::string& result)
{
    std::lock_guard<std::mutex> lock(cacheMutex);

    auto it = translationCache.find(key);

    if (it != translationCache.end())
    {
        result = it->second;
        return true;
    }

    return false;
}

/* ================================
   normailizeText - Utility to clean up input text
================================ */
std::string normalizeText(std::string s)
{
    // Trim left
    s.erase(0, s.find_first_not_of(" \t\n\r"));

    // Trim right
    s.erase(s.find_last_not_of(" \t\n\r") + 1);

    // Convert multiple spaces to single
    std::string out;
    bool space = false;

    for (char c : s)
    {
        if (isspace(c))
        {
            if (!space)
                out += ' ';
            space = true;
        }
        else
        {
            out += c;
            space = false;
        }
    }

    return out;
}

/* ================================
   makeCacheKey - Utility for caching translations
================================ */
std::string makeCacheKey(
    const std::string& text,
    const std::string& language)
{
    return normalizeText(text) + "||" + language;
}

/* ================================
   Main Server
================================ */

int main()
{
    try
    {
        /* Init curl */
        curl_global_init(CURL_GLOBAL_ALL);

        /* Read API key */
        GROQ_API_KEY = getApiKey();

        crow::SimpleApp app;

        /* ---------------------------
           Health Check
        ---------------------------- */

        CROW_ROUTE(app, "/health")
        ([](){
            return "OK";
        });


        /* ---------------------------
           Translate API
        ---------------------------- */

        CROW_ROUTE(app, "/api/translate")
        .methods("POST"_method)
        ([](const crow::request& req){

            try
            {
                auto body =
                    crow::json::load(req.body);

                if (!body)
                    return crow::response(
                        400, "Invalid JSON");

                if (!body.has("text") ||
                    !body.has("language"))
                    return crow::response(
                        400, "Missing fields");

                std::string text =
                    body["text"].s();

                std::string language =
                    body["language"].s();

                if (text.empty() ||
                    language.empty())
                    return crow::response(
                        400, "Empty values");

                #if 0
                /*
                Add caching layer 
                */
               // Normalize
                text = normalizeText(text);    
                // Create cache key
                std::string key = makeCacheKey(text, language);
                std::string cachedResult;
                // 1️. Check cache
                if (getFromCache(key, cachedResult))
                {
                  json res;

                  res["status"] = "ok";
                  res["result"] = cachedResult;
                  res["cached"] = true;

                  return crow::response(res.dump());
                }
                #endif

                //actual translation thru LLM
                std::string result =
                    translateText(text, language);
                // 2️. Save to cache
                //saveToCache(key, result);

                crow::json::wvalue resp;

                resp["status"] = "ok";
                resp["result"] = result;

                return crow::response(resp);
            }
            catch (std::exception& e)
            {
                std::cerr
                    << "ERROR: "
                    << e.what()
                    << std::endl;

                return crow::response(
                    500,
                    std::string("Server error: ")
                    + e.what());
            }
        });


        /* ---------------------------
           Run Server
        ---------------------------- */

        std::cout
            << "Server running at:\n"
            << "http://localhost:8080\n"
            << std::endl;

        // app.port(8080)
        //    .multithreaded()
        //    .run();

        // need to manage dynamic port for deployment platforms like Railway, Heroku, etc.
        const char *portEnv = std::getenv("PORT");

        int port = 8080; // fallback for local

        if (portEnv != nullptr)
        {
          port = std::stoi(portEnv);
        }

        std::cout << "Server starting on port " << port << std::endl;

        app.port(port)
            .multithreaded()
            .run();


        curl_global_cleanup();
    }
    catch (std::exception& e)
    {
        std::cerr
            << "Startup error: "
            << e.what()
            << std::endl;
    }

    return 0;
}
