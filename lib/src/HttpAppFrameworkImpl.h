/**
 *
 *  HttpAppFrameworkImpl.h
 *  An Tao
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  https://github.com/an-tao/drogon
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Drogon
 *
 */

#pragma once

#include "impl_forwards.h"
#include <drogon/HttpAppFramework.h>
#include <drogon/config.h>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <vector>
#include <functional>
#include <limits>

namespace drogon
{
struct InitBeforeMainFunction
{
    explicit InitBeforeMainFunction(const std::function<void()> &func)
    {
        func();
    }
};
class HttpAppFrameworkImpl : public HttpAppFramework
{
  public:
    HttpAppFrameworkImpl();

    virtual const Json::Value &getCustomConfig() const override
    {
        return jsonConfig_["custom_config"];
    }

    virtual PluginBase *getPlugin(const std::string &name) override;
    virtual HttpAppFramework &addListener(
        const std::string &ip,
        uint16_t port,
        bool useSSL = false,
        const std::string &certFile = "",
        const std::string &keyFile = "") override;
    virtual HttpAppFramework &setThreadNum(size_t threadNum) override;
    virtual size_t getThreadNum() const override
    {
        return threadNum_;
    }
    virtual HttpAppFramework &setSSLFiles(const std::string &certPath,
                                          const std::string &keyPath) override;
    virtual void run() override;
    virtual HttpAppFramework &registerWebSocketController(
        const std::string &pathName,
        const std::string &crtlName,
        const std::vector<std::string> &filters =
            std::vector<std::string>()) override;
    virtual HttpAppFramework &registerHttpSimpleController(
        const std::string &pathName,
        const std::string &crtlName,
        const std::vector<internal::HttpConstraint> &filtersAndMethods =
            std::vector<internal::HttpConstraint>{}) override;

    virtual HttpAppFramework &setCustom404Page(const HttpResponsePtr &resp,
                                               bool set404) override
    {
        if (set404)
        {
            resp->setStatusCode(k404NotFound);
        }
        custom404_ = resp;
        return *this;
    }

    const HttpResponsePtr &getCustom404Page();

    virtual void forward(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        const std::string &hostString = "") override;

    void forward(const HttpRequestImplPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback,
                 const std::string &hostString);

    virtual HttpAppFramework &registerBeginningAdvice(
        const std::function<void()> &advice) override
    {
        beginningAdvices_.emplace_back(advice);
        return *this;
    }

    virtual HttpAppFramework &registerNewConnectionAdvice(
        const std::function<bool(const trantor::InetAddress &,
                                 const trantor::InetAddress &)> &advice)
        override
    {
        newConnectionAdvices_.emplace_back(advice);
        return *this;
    }
    virtual HttpAppFramework &registerSyncAdvice(
        const std::function<HttpResponsePtr(const HttpRequestPtr &)> &advice)
        override
    {
        syncAdvices_.emplace_back(advice);
        return *this;
    }
    virtual HttpAppFramework &registerPreRoutingAdvice(
        const std::function<void(const HttpRequestPtr &,
                                 AdviceCallback &&,
                                 AdviceChainCallback &&)> &advice) override
    {
        preRoutingAdvices_.emplace_back(advice);
        return *this;
    }
    virtual HttpAppFramework &registerPostRoutingAdvice(
        const std::function<void(const HttpRequestPtr &,
                                 AdviceCallback &&,
                                 AdviceChainCallback &&)> &advice) override
    {
        postRoutingAdvices_.emplace_back(advice);
        return *this;
    }
    virtual HttpAppFramework &registerPreHandlingAdvice(
        const std::function<void(const HttpRequestPtr &,
                                 AdviceCallback &&,
                                 AdviceChainCallback &&)> &advice) override
    {
        preHandlingAdvices_.emplace_back(advice);
        return *this;
    }

    virtual HttpAppFramework &registerPreRoutingAdvice(
        const std::function<void(const HttpRequestPtr &)> &advice) override
    {
        preRoutingObservers_.emplace_back(advice);
        return *this;
    }
    virtual HttpAppFramework &registerPostRoutingAdvice(
        const std::function<void(const HttpRequestPtr &)> &advice) override
    {
        postRoutingObservers_.emplace_back(advice);
        return *this;
    }
    virtual HttpAppFramework &registerPreHandlingAdvice(
        const std::function<void(const HttpRequestPtr &)> &advice) override
    {
        preHandlingObservers_.emplace_back(advice);
        return *this;
    }
    virtual HttpAppFramework &registerPostHandlingAdvice(
        const std::function<void(const HttpRequestPtr &,
                                 const HttpResponsePtr &)> &advice) override
    {
        postHandlingAdvices_.emplace_back(advice);
        return *this;
    }

    virtual HttpAppFramework &enableSession(const size_t timeout = 0) override
    {
        useSession_ = true;
        sessionTimeout_ = timeout;
        return *this;
    }
    virtual HttpAppFramework &disableSession() override
    {
        useSession_ = false;
        return *this;
    }
    virtual const std::string &getDocumentRoot() const override
    {
        return rootPath_;
    }
    virtual HttpAppFramework &setDocumentRoot(
        const std::string &rootPath) override
    {
        rootPath_ = rootPath;
        return *this;
    }

    virtual HttpAppFramework &setStaticFileHeaders(
        const std::vector<std::pair<std::string, std::string>> &headers)
        override
    {
        staticFileHeaders_ = headers;
        return *this;
    }
    virtual const std::string &getUploadPath() const override
    {
        return uploadPath_;
    }
    virtual const std::shared_ptr<trantor::Resolver> &getResolver()
        const override
    {
        static auto resolver = trantor::Resolver::newResolver(getLoop());
        return resolver;
    }
    virtual HttpAppFramework &setUploadPath(
        const std::string &uploadPath) override;
    virtual HttpAppFramework &setFileTypes(
        const std::vector<std::string> &types) override;
    virtual HttpAppFramework &enableDynamicViewsLoading(
        const std::vector<std::string> &libPaths) override;
    virtual HttpAppFramework &setMaxConnectionNum(
        size_t maxConnections) override;
    virtual HttpAppFramework &setMaxConnectionNumPerIP(
        size_t maxConnectionsPerIP) override;
    virtual HttpAppFramework &loadConfigFile(
        const std::string &fileName) override;
    virtual HttpAppFramework &enableRunAsDaemon() override
    {
        runAsDaemon_ = true;
        return *this;
    }
    virtual HttpAppFramework &enableRelaunchOnError() override
    {
        relaunchOnError_ = true;
        return *this;
    }
    virtual HttpAppFramework &setLogPath(
        const std::string &logPath,
        const std::string &logfileBaseName = "",
        size_t logfileSize = 100000000) override;
    virtual HttpAppFramework &setLogLevel(
        trantor::Logger::LogLevel level) override;
    virtual HttpAppFramework &enableSendfile(bool sendFile) override
    {
        useSendfile_ = sendFile;
        return *this;
    }
    virtual HttpAppFramework &enableGzip(bool useGzip) override
    {
        useGzip_ = useGzip;
        return *this;
    }
    virtual bool isGzipEnabled() const override
    {
        return useGzip_;
    }
    virtual HttpAppFramework &setStaticFilesCacheTime(int cacheTime) override;
    virtual int staticFilesCacheTime() const override;
    virtual HttpAppFramework &setIdleConnectionTimeout(size_t timeout) override
    {
        idleConnectionTimeout_ = timeout;
        return *this;
    }
    virtual HttpAppFramework &setKeepaliveRequestsNumber(
        const size_t number) override
    {
        keepaliveRequestsNumber_ = number;
        return *this;
    }
    virtual HttpAppFramework &setPipeliningRequestsNumber(
        const size_t number) override
    {
        pipeliningRequestsNumber_ = number;
        return *this;
    }
    virtual HttpAppFramework &setGzipStatic(bool useGzipStatic) override;
    virtual HttpAppFramework &setClientMaxBodySize(size_t maxSize) override
    {
        clientMaxBodySize_ = maxSize;
        return *this;
    }
    virtual HttpAppFramework &setClientMaxMemoryBodySize(
        size_t maxSize) override
    {
        clientMaxMemoryBodySize_ = maxSize;
        return *this;
    }
    virtual HttpAppFramework &setClientMaxWebSocketMessageSize(
        size_t maxSize) override
    {
        clientMaxWebSocketMessageSize_ = maxSize;
        return *this;
    }
    virtual HttpAppFramework &setHomePage(
        const std::string &homePageFile) override
    {
        homePageFile_ = homePageFile;
        return *this;
    }
    const std::string &getHomePage() const
    {
        return homePageFile_;
    }
    size_t getClientMaxBodySize() const
    {
        return clientMaxBodySize_;
    }
    size_t getClientMaxMemoryBodySize() const
    {
        return clientMaxMemoryBodySize_;
    }
    size_t getClientMaxWebSocketMessageSize() const
    {
        return clientMaxWebSocketMessageSize_;
    }
    virtual std::vector<std::tuple<std::string, HttpMethod, std::string>>
    getHandlersInfo() const override;

    size_t keepaliveRequestsNumber() const
    {
        return keepaliveRequestsNumber_;
    }
    size_t pipeliningRequestsNumber() const
    {
        return pipeliningRequestsNumber_;
    }

    virtual ~HttpAppFrameworkImpl() noexcept;
    virtual bool isRunning() override
    {
        return running_;
    }

    virtual trantor::EventLoop *getLoop() const override;

    virtual void quit() override;

    virtual HttpAppFramework &setServerHeaderField(
        const std::string &server) override
    {
        assert(!running_);
        assert(server.find("\r\n") == std::string::npos);
        serverHeader_ = "Server: " + server + "\r\n";
        return *this;
    }

    virtual HttpAppFramework &enableServerHeader(bool flag) override
    {
        enableServerHeader_ = flag;
        return *this;
    }
    virtual HttpAppFramework &enableDateHeader(bool flag) override
    {
        enableDateHeader_ = flag;
        return *this;
    }
    bool sendServerHeader() const
    {
        return enableServerHeader_;
    }
    bool sendDateHeader() const
    {
        return enableDateHeader_;
    }
    const std::string &getServerHeaderString() const
    {
        return serverHeader_;
    }

    virtual orm::DbClientPtr getDbClient(
        const std::string &name = "default") override;
    virtual orm::DbClientPtr getFastDbClient(
        const std::string &name = "default") override;
    virtual HttpAppFramework &createDbClient(
        const std::string &dbType,
        const std::string &host,
        const unsigned short port,
        const std::string &databaseName,
        const std::string &userName,
        const std::string &password,
        const size_t connectionNum = 1,
        const std::string &filename = "",
        const std::string &name = "default",
        const bool isFast = false) override;

    inline static HttpAppFrameworkImpl &instance()
    {
        static HttpAppFrameworkImpl instance;
        return instance;
    }
    bool useSendfile()
    {
        return useSendfile_;
    }
    void callCallback(
        const HttpRequestImplPtr &req,
        const HttpResponsePtr &resp,
        const std::function<void(const HttpResponsePtr &)> &callback);

    virtual bool supportSSL() const override
    {
#ifdef OpenSSL_FOUND
        return true;
#endif
        return false;
    }

    virtual size_t getCurrentThreadIndex() const override
    {
        auto *loop = trantor::EventLoop::getEventLoopOfCurrentThread();
        if (loop)
        {
            return loop->index();
        }
        return std::numeric_limits<size_t>::max();
    }

  private:
    virtual void registerHttpController(
        const std::string &pathPattern,
        const internal::HttpBinderBasePtr &binder,
        const std::vector<HttpMethod> &validMethods = std::vector<HttpMethod>(),
        const std::vector<std::string> &filters = std::vector<std::string>(),
        const std::string &handlerName = "") override;
    void onAsyncRequest(
        const HttpRequestImplPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback);
    void onNewWebsockRequest(
        const HttpRequestImplPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        const WebSocketConnectionImplPtr &wsConnPtr);
    void onConnection(const trantor::TcpConnectionPtr &conn);
    void addHttpPath(const std::string &path,
                     const internal::HttpBinderBasePtr &binder,
                     const std::vector<HttpMethod> &validMethods,
                     const std::vector<std::string> &filters);

    // We use a uuid string as session id;
    // set sessionTimeout_=0 to make location session valid forever based on
    // cookies;
    size_t sessionTimeout_{0};
    size_t idleConnectionTimeout_{60};
    bool useSession_{false};
    std::string serverHeader_{"Server: drogon/" + drogon::getVersion() +
                              "\r\n"};

    const std::unique_ptr<StaticFileRouter> staticFileRouterPtr_;
    const std::unique_ptr<HttpControllersRouter> httpCtrlsRouterPtr_;
    const std::unique_ptr<HttpSimpleControllersRouter>
        httpSimpleCtrlsRouterPtr_;
    const std::unique_ptr<WebsocketControllersRouter> websockCtrlsRouterPtr_;

    const std::unique_ptr<ListenerManager> listenerManagerPtr_;
    const std::unique_ptr<PluginsManager> pluginsManagerPtr_;
    const std::unique_ptr<orm::DbClientManager> dbClientManagerPtr_;

    std::string rootPath_{"./"};
    std::vector<std::pair<std::string, std::string>> staticFileHeaders_;
    std::string uploadPath_;
    std::atomic_bool running_{false};

    size_t threadNum_{1};
    std::vector<std::string> libFilePaths_;

    std::unique_ptr<SharedLibManager> sharedLibManagerPtr_;

    std::string sslCertPath_;
    std::string sslKeyPath_;

    size_t maxConnectionNumPerIP_{0};
    std::unordered_map<std::string, size_t> connectionsNumMap_;

    int64_t maxConnectionNum_{100000};
    std::atomic<int64_t> connectionNum_{0};

    bool runAsDaemon_{false};
    bool relaunchOnError_{false};
    std::string logPath_{""};
    std::string logfileBaseName_{""};
    size_t logfileSize_{100000000};
    size_t keepaliveRequestsNumber_{0};
    size_t pipeliningRequestsNumber_{0};
    bool useSendfile_{true};
    bool useGzip_{true};
    size_t clientMaxBodySize_{1024 * 1024};
    size_t clientMaxMemoryBodySize_{64 * 1024};
    size_t clientMaxWebSocketMessageSize_{128 * 1024};
    std::string homePageFile_{"index.html"};
    std::unique_ptr<SessionManager> sessionManagerPtr_;
    Json::Value jsonConfig_;
    HttpResponsePtr custom404_;
    static InitBeforeMainFunction initFirst_;
    bool enableServerHeader_{true};
    bool enableDateHeader_{true};
    std::vector<std::function<void()>> beginningAdvices_;
    std::vector<std::function<bool(const trantor::InetAddress &,
                                   const trantor::InetAddress &)>>
        newConnectionAdvices_;
    std::vector<std::function<HttpResponsePtr(const HttpRequestPtr &)>>
        syncAdvices_;
    std::vector<std::function<void(const HttpRequestPtr &,
                                   AdviceCallback &&,
                                   AdviceChainCallback &&)>>
        preRoutingAdvices_;
    std::vector<std::function<void(const HttpRequestPtr &,
                                   AdviceCallback &&,
                                   AdviceChainCallback &&)>>
        postRoutingAdvices_;
    std::vector<std::function<void(const HttpRequestPtr &,
                                   AdviceCallback &&,
                                   AdviceChainCallback &&)>>
        preHandlingAdvices_;
    std::vector<
        std::function<void(const HttpRequestPtr &, const HttpResponsePtr &)>>
        postHandlingAdvices_;

    std::vector<std::function<void(const HttpRequestPtr &)>>
        preRoutingObservers_;
    std::vector<std::function<void(const HttpRequestPtr &)>>
        postRoutingObservers_;
    std::vector<std::function<void(const HttpRequestPtr &)>>
        preHandlingObservers_;
};

}  // namespace drogon
