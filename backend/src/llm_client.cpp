#include "llm_client.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#if 0

using json = nlohmann::json;

// Curl response handler
static size_t writeCB(
    void* ptr, size_t size,
    size_t nmemb, void* data)
{
    ((std::string*)data)
      ->append((char*)ptr, size*nmemb);

    return size*nmemb;
}

LLMClient::LLMClient(){

    apiKey = "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"; // move to env later

    apiUrl =
    "https://openrouter.ai/api/v1/chat/completions";

    curl_global_init(CURL_GLOBAL_ALL);
}

std::string LLMClient::sendPrompt(
    const std::string& prompt)
{
    CURL* curl = curl_easy_init();

    std::string response;

    json body;

    body["model"] =
      "meta-llama/llama-3-8b-instruct";

    body["messages"] = json::array({

        {
          {"role","system"},
          {"content","You are translator"}
        },
        {
          {"role","user"},
          {"content",prompt}
        }
    });

    body["temperature"] = 0.0;

    std::string postData = body.dump();

    struct curl_slist* headers=nullptr;

    headers = curl_slist_append(
      headers,"Content-Type: application/json");

    headers = curl_slist_append(
      headers,
      ("Authorization: Bearer "+apiKey).c_str());

    curl_easy_setopt(curl,
      CURLOPT_URL, apiUrl.c_str());

    curl_easy_setopt(curl,
      CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl,
      CURLOPT_POSTFIELDS, postData.c_str());

    curl_easy_setopt(curl,
      CURLOPT_WRITEFUNCTION, writeCB);

    curl_easy_setopt(curl,
      CURLOPT_WRITEDATA, &response);

    curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    // Parse response
    auto j = json::parse(response);

    return j["choices"][0]
            ["message"]
            ["content"];
}
#endif
