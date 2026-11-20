#include "RankingClient.h"
#include "Constants.h"
#include "Exceptions.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

namespace
{
    struct Handle
    {
        HINTERNET h;
        ~Handle()
        {
            if (h)
                WinHttpCloseHandle(h);
        }
    };

    // ponytail: so HTTP; para HTTPS passar WINHTTP_FLAG_SECURE no WinHttpOpenRequest
    std::string request(const wchar_t *method, const std::wstring &path, const std::string &body = "")
    {
        Handle session{WinHttpOpen(L"SpaceInvaders/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                   WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0)};
        if (!session.h)
            throw NetworkException("nao foi possivel iniciar o WinHTTP");
        WinHttpSetTimeouts(session.h, 3000, 3000, 3000, 3000);

        Handle conn{WinHttpConnect(session.h, Cfg::RANKING_HOST, Cfg::RANKING_PORT, 0)};
        Handle req{conn.h ? WinHttpOpenRequest(conn.h, method, path.c_str(), nullptr, WINHTTP_NO_REFERER,
                                               WINHTTP_DEFAULT_ACCEPT_TYPES, 0)
                          : nullptr};

        const wchar_t *headers = body.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : L"Content-Type: application/json\r\n";
        DWORD size = static_cast<DWORD>(body.size());
        if (!req.h ||
            !WinHttpSendRequest(req.h, headers, body.empty() ? 0 : DWORD(-1),
                                const_cast<char *>(body.data()), size, size, 0) ||
            !WinHttpReceiveResponse(req.h, nullptr))
            throw NetworkException("servidor de ranking indisponivel");

        DWORD status = 0, len = sizeof(status);
        WinHttpQueryHeaders(req.h, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &status, &len, WINHTTP_NO_HEADER_INDEX);

        std::string out;
        DWORD avail = 0;
        while (WinHttpQueryDataAvailable(req.h, &avail) && avail > 0)
        {
            std::string buf(avail, '\0');
            DWORD read = 0;
            if (!WinHttpReadData(req.h, buf.data(), avail, &read))
                break;
            out.append(buf, 0, read);
        }

        if (status < 200 || status >= 300)
            throw NetworkException("servidor respondeu " + std::to_string(status) + ": " + out);
        return out;
    }
}
#else
namespace
{
    // ponytail: ranking online so no Windows (WinHTTP); em Linux/macOS o jogo roda sem ranking
    std::string request(const wchar_t *, const std::wstring &, const std::string & = "")
    {
        throw NetworkException("ranking online disponivel apenas no Windows");
    }
}
#endif

namespace RankingClient
{
    void submitScore(const ScoreEntry &entry)
    {
        std::string name;
        for (char c : entry.playerName)
        {
            if (c == '"' || c == '\\')
                name += '\\';
            name += c;
        }
        request(L"POST", L"/scores",
                "{\"name\":\"" + name + "\",\"score\":" + std::to_string(entry.score) + "}");
    }

    std::vector<ScoreEntry> fetchTop(int n)
    {
        return parseScores(request(L"GET", L"/scores?limit=" + std::to_wstring(n)));
    }

    // ponytail: parser minimo para o formato fixo do nosso servidor, nao e JSON generico
    std::vector<ScoreEntry> parseScores(const std::string &json)
    {
        const std::string nameKey = "\"name\":\"", scoreKey = "\"score\":";
        std::vector<ScoreEntry> out;
        size_t pos = 0;
        while ((pos = json.find(nameKey, pos)) != std::string::npos)
        {
            pos += nameKey.size();
            ScoreEntry e;
            while (pos < json.size() && json[pos] != '"')
            {
                char c = json[pos++];
                if (c == '\\' && pos < json.size())
                {
                    c = json[pos++];
                    if (c == 'u') // \uXXXX: a fonte do jogo nao desenha mesmo
                    {
                        pos += 4;
                        c = '?';
                    }
                }
                e.playerName += c;
            }
            size_t s = json.find(scoreKey, pos);
            if (s == std::string::npos)
                throw NetworkException("resposta invalida do ranking");
            try
            {
                e.score = std::stoi(json.substr(s + scoreKey.size(), 12));
            }
            catch (const std::exception &)
            {
                throw NetworkException("resposta invalida do ranking");
            }
            pos = s;
            out.push_back(std::move(e));
        }
        return out;
    }
}
