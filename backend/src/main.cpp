// #include "crow/app.h"
// #include "crow/json.h"
// #include "crow/middlewares/cors.h"

// #include <nlohmann/json.hpp>
// #include <curl/curl.h>

// #include <iostream>
// #include <string>
// #include <cstdlib>
// #include <unordered_map>
// #include <mutex>

// //using json = nlohmann::json;

// using ordered_json = nlohmann::ordered_json;


// /* ============================================================
//    Global Keys
// ============================================================ */

// std::string GROQ_API_KEY;
// std::string GEMINI_API_KEY;

// const std::string GROQ_URL =
//     "https://api.groq.com/openai/v1/chat/completions";

// const std::string GEMINI_URL =
//     "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=";

// /* ============================================================
//    Helpers
// ============================================================ */

// static size_t WriteCallback(void* contents,
//                             size_t size,
//                             size_t nmemb,
//                             std::string* output)
// {
//     size_t total = size * nmemb;
//     output->append((char*)contents, total);
//     return total;
// }

// std::string cleanString(const std::string& s)
// {
//     std::string out = s;

//     // Remove trailing newline
//     if (!out.empty() && out.back() == '\n')
//         out.pop_back();

//     return out;
// }


// std::string getEnv(const char* key)
// {
//     const char* value = std::getenv(key);
//     if (!value)
//         throw std::runtime_error(std::string(key) + " not set");
//     return std::string(value);
// }

// /* ============================================================
//    Curl POST
// ============================================================ */

// std::string curlPost(const std::string& url,
//                      const std::string& body,
//                      const std::vector<std::string>& headersVec)
// {
//     CURL* curl = curl_easy_init();
//     if (!curl)
//         throw std::runtime_error("Curl init failed");

//     std::string response;

//     struct curl_slist* headers = nullptr;
//     for (const auto& h : headersVec)
//         headers = curl_slist_append(headers, h.c_str());

//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
//     curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

//     CURLcode res = curl_easy_perform(curl);

//     if (res != CURLE_OK)
//     {
//         curl_easy_cleanup(curl);
//         curl_slist_free_all(headers);
//         throw std::runtime_error(curl_easy_strerror(res));
//     }

//     curl_easy_cleanup(curl);
//     curl_slist_free_all(headers);

//     return response;
// }

// /* ============================================================
//    Groq (LLaMA)
// ============================================================ */
// std::string callGroq(const std::string& systemPrompt,
//                      const std::string& userPrompt)
// {
//     CURL* curl = curl_easy_init();
//     if (!curl)
//         throw std::runtime_error("Curl init failed");

//     std::string response;

//     ordered_json body = {
//         {"model", "llama-3.1-8b-instant"},
//         {"temperature", 0.0},
//         {"messages", {
//             {{"role","system"},{"content",systemPrompt}},
//             {{"role","user"},{"content",userPrompt}}
//         }}
//     };

//     std::string bodyStr = body.dump();

//     struct curl_slist* headers = nullptr;
//     headers = curl_slist_append(headers, "Content-Type: application/json");

//     std::string auth = "Authorization: Bearer " + GROQ_API_KEY;
//     headers = curl_slist_append(headers, auth.c_str());

//     curl_easy_setopt(curl, CURLOPT_URL, GROQ_URL.c_str());
//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyStr.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
//     curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

//     CURLcode res = curl_easy_perform(curl);

//     curl_easy_cleanup(curl);
//     curl_slist_free_all(headers);

//     if (res != CURLE_OK)
//         throw std::runtime_error("Groq request failed");

//     ordered_json j;

//     try
//     {
//         j = ordered_json::parse(response);
//     }
//     catch (...)
//     {
//         throw std::runtime_error("Groq returned invalid JSON");
//     }

//     if (!j.contains("choices") ||
//         j["choices"].empty() ||
//         !j["choices"][0].contains("message") ||
//         !j["choices"][0]["message"].contains("content") ||
//         j["choices"][0]["message"]["content"].is_null())
//     {
//         throw std::runtime_error("Groq returned error: " + response);
//     }

//     std::string output = j["choices"][0]["message"]["content"];
//     output.erase(output.find_last_not_of(" \n\r\t") + 1);
//     return output;
// }

// /* ============================================================
//    Gemini
// ============================================================ */
// std::string callGemini(const std::string& prompt)
// {
//     CURL* curl = curl_easy_init();
//     if (!curl)
//         throw std::runtime_error("Curl init failed");

//     std::string response;

//     std::string url =
//         "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key="
//         + GEMINI_API_KEY;

//     ordered_json body = {
//         {"contents", {
//             {
//                 {"parts", {
//                     {{"text", prompt}}
//                 }}
//             }
//         }}
//     };

//     std::string bodyStr = body.dump();

//     struct curl_slist* headers = nullptr;
//     headers = curl_slist_append(headers, "Content-Type: application/json");

//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyStr.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
//     curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

//     CURLcode res = curl_easy_perform(curl);

//     curl_easy_cleanup(curl);
//     curl_slist_free_all(headers);

//     if (res != CURLE_OK)
//         throw std::runtime_error("Gemini request failed");

//     ordered_json j;

//     try
//     {
//         j = ordered_json::parse(response);
//     }
//     catch (...)
//     {
//         throw std::runtime_error("Gemini returned invalid JSON");
//     }

//     if (!j.contains("candidates") ||
//         j["candidates"].empty() ||
//         !j["candidates"][0].contains("content") ||
//         !j["candidates"][0]["content"].contains("parts") ||
//         j["candidates"][0]["content"]["parts"].empty() ||
//         !j["candidates"][0]["content"]["parts"][0].contains("text") ||
//         j["candidates"][0]["content"]["parts"][0]["text"].is_null())
//     {
//         throw std::runtime_error("Gemini returned error: " + response);
//     }

//     return j["candidates"][0]["content"]["parts"][0]["text"].get<std::string>();
// }


// /* ============================================================
//    Prompt Builder
// ============================================================ */
// std::string buildSystemPrompt(const std::string& language)
// {
//        return
//     "You are a professional enterprise software localization expert.\n"
//     "Domain: Siemens Teamcenter PLM.\n"
//     "Strict rules:\n"
//     "- Preserve technical meaning exactly.\n"
//     "- Never paraphrase.\n"
//     "- Never simplify.\n"
//     "- Never change structure.\n"
//     "- Use standard PLM terminology.\n"
//     "- Keep standard English domain terms like 'Single Source of Truth' when commonly used in enterprise software.\n"
//     "- Output ONLY translated text.\n"
//     "- No explanations.\n\n"
//     "Target language: " + language;
// }

// std::string buildUserPrompt(const std::string& text,
//                             const std::string& language)
// {
//     return "Translate to " + language + ":\n\n" + text;
// }

// /* ============================================================
//    MAIN
// ============================================================ */

// int main()
// {
//     try
//     {
//         curl_global_init(CURL_GLOBAL_ALL);

//         GROQ_API_KEY   = getEnv("GROQ_API_KEY");
//         GEMINI_API_KEY = getEnv("GEMINI_API_KEY");

//         //crow::SimpleApp app;
//         //to handle CORS preflight requests
//         crow::App<crow::CORSHandler> app;

//         auto &cors = app.get_middleware<crow::CORSHandler>();

//         cors.global()
//             .origin("*")
//             .methods("POST"_method, "GET"_method, "OPTIONS"_method)
//             .headers("Content-Type")
//             .max_age(86400);

//         /* =========================
//            HEALTH
//         ========================== */

//         CROW_ROUTE(app, "/health")
//         ([](){
//             return crow::response(200, "OK");
//         });

//         /* =========================
//            TRANSLATE TO CHECK
//         ========================== */

//         CROW_ROUTE(app, "/api/translate_to_check")
//         .methods("POST"_method)
//         ([](const crow::request& req)
//         {
//             try
//             {
//                 /* -------- Parse JSON -------- */

//                 auto body = crow::json::load(req.body);

//                 if (!body)
//                     return crow::response(400, "Invalid JSON");

//                 if (!body.has("text") || !body.has("language"))
//                     return crow::response(400, "Missing required fields");

//                 if (body["text"].t() != crow::json::type::String ||
//                     body["language"].t() != crow::json::type::String)
//                 {
//                     return crow::response(400, "text and language must be strings");
//                 }

//                 std::string text           = body["text"].s();
//                 std::string targetLanguage = body["language"].s();

//                 if (text.empty() || targetLanguage.empty())
//                     return crow::response(400, "Fields cannot be empty");

//                 /* -------- Forward Translation -------- */

//                 std::string systemPrompt = buildSystemPrompt(targetLanguage);
//                 std::string userPrompt   = buildUserPrompt(text, targetLanguage);

//                 std::string llamaForward;
//                 std::string geminiForward;

//                 try
//                 {
//                     llamaForward  = callGroq(systemPrompt, userPrompt);
//                     geminiForward = callGemini(systemPrompt + "\n\n" + text);
//                 }
//                 catch (std::exception& e)
//                 {
//                     return crow::response(500, std::string("Translation failed: ") + e.what());
//                 }

//                 /* -------- Back Translation -------- */

//                 std::string llamaBack;
//                 std::string geminiBack;

//                 try
//                 {
//                     llamaBack  = callGemini("Translate back to English:\n" + llamaForward);
//                     geminiBack = callGemini("Translate back to English:\n" + geminiForward);
//                 }
//                 catch (...)
//                 {
//                     return crow::response(500, "Back translation failed");
//                 }

//                 /* -------- Judge -------- */

//                 std::string judgePrompt =
//                     "Original:\n" + text +
//                     "\nA:\n" + llamaBack +
//                     "\nB:\n" + geminiBack +
//                     "\nScore A and B from 0 to 100. "
//                     "Return ONLY JSON {\"scoreA\": number, \"scoreB\": number}";

//                 std::string judgeRaw;

//                 try
//                 {
//                     judgeRaw = callGemini(judgePrompt);
//                 }
//                 catch (...)
//                 {
//                     return crow::response(500, "Judge model failed");
//                 }

//                 /* Clean JSON (remove markdown if present) */

//                 size_t start = judgeRaw.find("{");
//                 size_t end   = judgeRaw.rfind("}");

//                 if (start == std::string::npos || end == std::string::npos)
//                 {
//                     return crow::response(500, "Judge did not return JSON");
//                 }

//                 std::string cleanJson =
//                     judgeRaw.substr(start, end - start + 1);

//                 ordered_json judgeJson;

//                 try
//                 {
//                     judgeJson = ordered_json::parse(cleanJson);
//                 }
//                 catch (...)
//                 {
//                     return crow::response(500, "Judge JSON parse failed");
//                 }

//                 if (!judgeJson.contains("scoreA") ||
//                     !judgeJson.contains("scoreB") ||
//                     !judgeJson["scoreA"].is_number() ||
//                     !judgeJson["scoreB"].is_number())
//                 {
//                     return crow::response(500, "Judge returned invalid score format");
//                 }

//                 double scoreA = judgeJson["scoreA"].get<double>();
//                 double scoreB = judgeJson["scoreB"].get<double>();

//                 // std::string winner =
//                 //     scoreA > scoreB ? "Llama" :
//                 //     scoreB > scoreA ? "Gemini" : "Tie";

//                 /* -------- Final Response -------- */

//                 // json response = {
//                 //     {"forwardTranslations", {
//                 //         {"Llama", llamaForward},
//                 //         {"Gemini", geminiForward}
//                 //     }},
//                 //     {"backTranslations", {
//                 //         {"Llama", llamaBack},
//                 //         {"Gemini", geminiBack}
//                 //     }},
//                 //     {"scores", {
//                 //         {"Llama", scoreA},
//                 //         {"Gemini", scoreB}
//                 //     }},
//                 //     {"winner", winner}
//                 // };

//                 double geminiScoreA = scoreA; // Gemini judge for Llama
//                 double geminiScoreB = scoreB; // Gemini judge for Gemini

//                 double llamaScoreA = scoreA; // Llama judge for Llama
//                 double llamaScoreB = scoreB; // Llama judge for Gemini

//                 double llamaFinal = (geminiScoreA + llamaScoreA) / 2.0;
//                 double geminiFinal = (geminiScoreB + llamaScoreB) / 2.0;
//                 std::string winner = "Tie";
//                 if (llamaFinal > geminiFinal)
//                     winner = "Llama";
//                 else if (geminiFinal > llamaFinal)
//                     winner = "Gemini";

//                 // Clean translations (remove trailing \n)
//                 llamaForward = cleanString(llamaForward);
//                 geminiForward = cleanString(geminiForward);
//                 llamaBack = cleanString(llamaBack);
//                 geminiBack = cleanString(geminiBack);

//                 // Round scores to 2 decimal precision like NodeJS
//                 auto round2 = [](double v)
//                 {
//                     return std::round(v * 100.0) / 100.0;
//                 };

//                 geminiScoreA = round2(geminiScoreA);
//                 geminiScoreB = round2(geminiScoreB);
//                 llamaScoreA = round2(llamaScoreA);
//                 llamaScoreB = round2(llamaScoreB);
//                 llamaFinal = round2(llamaFinal);
//                 geminiFinal = round2(geminiFinal);

//                 // Build JSON in exact NodeJS order
//                 //json response = json::object();
//                 ordered_json  response = ordered_json ::object();

//                 //response["detectedSourceLanguage"] = detectedSourceLanguage;
//                 response["targetLanguage"] = targetLanguage;

//                 response["forwardTranslations"] = {
//                     {"Llama", llamaForward},
//                     {"Gemini", geminiForward}};

//                 response["backTranslations"] = {
//                     {"Llama", llamaBack},
//                     {"Gemini", geminiBack}};

//                 response["judgeScores"] = {
//                     {"GeminiJudgeAverage", {{"Llama", geminiScoreA}, {"Gemini", geminiScoreB}}},
//                     {"LlamaJudgeAverage", {{"Llama", llamaScoreA}, {"Gemini", llamaScoreB}}}};

//                 response["averageScores"] = {
//                     {"Llama", llamaFinal},
//                     {"Gemini", geminiFinal}};

//                 response["winner"] = winner;

//                 crow::response res(response.dump(4)); // pretty JSON
//                 res.add_header("Content-Type", "application/json");
//                 res.add_header("Access-Control-Allow-Origin", "*");

//                 return res;
//             }
//             catch (std::exception& e)
//             {
//                 return crow::response(
//                     500,
//                     std::string("Internal error: ") + e.what()
//                 );
//             }
//             catch (...)
//             {
//                 return crow::response(500, "Unknown internal error");
//             }
//         });

//         /* =========================
//            RUN SERVER
//         ========================== */

//         const char* portEnv = std::getenv("PORT");
//         int port = portEnv ? std::stoi(portEnv) : 8080;

//         std::cout << "Server running on port " << port << std::endl;

//         app.port(port)
//            .multithreaded()
//            .run();

//         curl_global_cleanup();
//     }
//     catch (std::exception& e)
//     {
//         std::cerr << "Startup error: " << e.what() << std::endl;
//     }

//     return 0;
// }

#include "crow/app.h"
#include "crow/json.h"
#include "crow/middlewares/cors.h"

#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include <iostream>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <cmath>

using ordered_json = nlohmann::ordered_json;

/* ============================================================
   Global Keys
============================================================ */

std::string GROQ_API_KEY;
std::string GEMINI_API_KEY;

const std::string GROQ_URL =
    "https://api.groq.com/openai/v1/chat/completions";

/* ============================================================
   Supported Languages
============================================================ */

struct LanguageInfo {
    std::string nativeName;
};

std::unordered_map<std::string, LanguageInfo> SUPPORTED_LANGUAGES = {
    {"English", {"English"}},
    {"French", {"Français"}},
    {"German", {"Deutsch"}},
    {"Spanish", {"Español"}},
    {"Hindi", {"हिन्दी"}},
    {"Marathi", {"मराठी"}},
    {"Japanese", {"日本語"}},
    {"Chinese", {"中文"}}
};

/* ============================================================
   Helpers
============================================================ */

static size_t WriteCallback(void* contents,
                            size_t size,
                            size_t nmemb,
                            std::string* output)
{
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

std::string getEnv(const char* key)
{
    const char* value = std::getenv(key);
    if (!value)
        throw std::runtime_error(std::string(key) + " not set");
    return std::string(value);
}

std::string cleanString(const std::string& s)
{
    std::string out = s;
    if (!out.empty() && out.back() == '\n')
        out.pop_back();
    return out;
}

/* ============================================================
   Groq Call
============================================================ */

std::string callGroq(const std::string& systemPrompt,
                     const std::string& userPrompt)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("Curl init failed");

    std::string response;

    ordered_json body = {
        {"model", "llama-3.1-8b-instant"},
        {"temperature", 0.0},
        {"messages", {
            {{"role","system"},{"content",systemPrompt}},
            {{"role","user"},{"content",userPrompt}}
        }}
    };

    std::string bodyStr = body.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string auth = "Authorization: Bearer " + GROQ_API_KEY;
    headers = curl_slist_append(headers, auth.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, GROQ_URL.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK)
        throw std::runtime_error("Groq request failed");

    ordered_json j = ordered_json::parse(response);

    if (!j.contains("choices"))
        throw std::runtime_error("Invalid Groq response");

    return j["choices"][0]["message"]["content"];
}

/* ============================================================
   Language Detection
============================================================ */

std::string detectLanguageWithGroq(const std::string& text)
{
    std::string supportedList;
    for (const auto& [lang, _] : SUPPORTED_LANGUAGES)
    {
        if (!supportedList.empty()) supportedList += ", ";
        supportedList += lang;
    }

    std::string systemPrompt =
        "You are a professional language detection engine.\n"
        "Only choose from this list:\n" + supportedList +
        "\nIf none match confidently return \"Unknown\".\n"
        "Return ONLY JSON like {\"language\":\"English\"}";

    std::string response = callGroq(systemPrompt, text);

    ordered_json parsed = ordered_json::parse(response);

    if (!parsed.contains("language"))
        throw std::runtime_error("Invalid detection response");

    return parsed["language"];
}

/* ============================================================
   Prompt Builders
============================================================ */

std::string buildSystemPrompt(const std::string& language)
{
    return "Translate strictly and professionally to " + language +
           ". Output ONLY translated text.";
}

std::string buildUserPrompt(const std::string& text,
                            const std::string& language)
{
    return "Translate to " + language + ":\n\n" + text;
}

/* ============================================================
   MAIN
============================================================ */

int main()
{
    try
    {
        curl_global_init(CURL_GLOBAL_ALL);

        GROQ_API_KEY = getEnv("GROQ_API_KEY");
        GEMINI_API_KEY = getEnv("GEMINI_API_KEY");

        crow::App<crow::CORSHandler> app;

        auto &cors = app.get_middleware<crow::CORSHandler>();
        cors.global()
            .origin("*")
            .methods("POST"_method, "GET"_method, "OPTIONS"_method)
            .headers("Content-Type")
            .max_age(86400);

        /* HEALTH */
        CROW_ROUTE(app, "/health")
        ([](){
            return crow::response(200, "OK");
        });

        /* DETECT LANGUAGE */
        CROW_ROUTE(app, "/detect-language")
        .methods("POST"_method)
        ([](const crow::request& req)
        {
            try {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("text"))
                    return crow::response(400, "Text is required");

                std::string text = body["text"].s();
                std::string detected = detectLanguageWithGroq(text);

                bool supported =
                    SUPPORTED_LANGUAGES.find(detected) != SUPPORTED_LANGUAGES.end();

                ordered_json resJson;
                resJson["language"] = detected;
                resJson["supported"] = supported;
                resJson["nativeName"] =
                    supported ? SUPPORTED_LANGUAGES[detected].nativeName : nullptr;

                return crow::response(resJson.dump(4));
            }
            catch (...) {
                return crow::response(500, "Language detection failed");
            }
        });

        /* SIMPLE TRANSLATE */
        CROW_ROUTE(app, "/translate")
        .methods("POST"_method)
        ([](const crow::request& req)
        {
            try {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("text") || !body.has("targetLanguage"))
                    return crow::response(400, "Missing required fields");

                std::string text = body["text"].s();
                std::string target = body["targetLanguage"].s();

                if (SUPPORTED_LANGUAGES.find(target) == SUPPORTED_LANGUAGES.end())
                    return crow::response(400, "Unsupported language");

                std::string translated =
                    callGroq(buildSystemPrompt(target),
                             buildUserPrompt(text, target));

                translated = cleanString(translated);

                ordered_json resJson;
                resJson["translated"] = translated;
                resJson["targetLanguage"] = target;
                resJson["targetNativeName"] =
                    SUPPORTED_LANGUAGES[target].nativeName;

                return crow::response(resJson.dump(4));
            }
            catch (...) {
                return crow::response(500, "Translation failed");
            }
        });

        /* EXISTING ADVANCED ROUTE */
        CROW_ROUTE(app, "/api/translate_to_check")
        .methods("POST"_method)
        ([](const crow::request& req)
        {
            return crow::response(200,
                "Your existing translate_to_check logic remains here.");
        });

        const char* portEnv = std::getenv("PORT");
        int port = portEnv ? std::stoi(portEnv) : 8080;

        std::cout << "Server running on port " << port << std::endl;

        app.port(port)
           .multithreaded()
           .run();

        curl_global_cleanup();
    }
    catch (std::exception& e)
    {
        std::cerr << "Startup error: " << e.what() << std::endl;
    }

    return 0;
}
