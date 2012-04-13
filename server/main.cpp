#include "handlers/MediaServiceHandler.h"
#include "handlers/MediaSessionServiceHandler.h"
#include "handlers/NetworkConnectionServiceHandler.h"
#include "handlers/MixerServiceHandler.h"

#include <protocol/TBinaryProtocol.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <server/TThreadedServer.h>
#include <concurrency/PosixThreadFactory.h>
#include <concurrency/ThreadManager.h>

#include <glibmm.h>

#include "log.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using namespace ::com::kurento::kms::api;

using boost::shared_ptr;
using com::kurento::kms::MediaServerServiceHandler;
using com::kurento::kms::MediaSessionServiceHandler;
using com::kurento::kms::NetworkConnectionServiceHandler;
using com::kurento::kms::MixerServiceHandler;
using com::kurento::kms::api::ServerConfig;
using ::com::kurento::log::Log;

static Log l("main");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

#define SERVER_ADDRESS "localhost"
#define SERVER_SERVICE_PORT 9090
#define SESSION_SERVICE_PORT 9091
#define NETWORK_CONNECTION_SERVICE_PORT 9092
#define MIXER_SERVICE_PORT 9093

static ServerConfig config;

static void create_server_service() {
	int port;

	port = config.serverServicePort;

	shared_ptr<MediaServerServiceHandler> handler(new MediaServerServiceHandler(&config));
	shared_ptr<TProcessor> processor(new MediaServerServiceProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory ());

	shared_ptr<ThreadManager> threadManager= ThreadManager::newSimpleThreadManager(15);

	threadManager->threadFactory(threadFactory);

	shared_ptr<TThreadedServer> server(new TThreadedServer(processor,
					serverTransport, transportFactory,
					protocolFactory, threadFactory));

	i("Starting MediaServerService");
	server->serve();

	i("MediaServerService stopped finishing thread");
	throw Glib::Thread::Exit();
}

static void create_session_service() {
	int port;

	if (!config.__isset.mediaSessionServicePort) {
		w("No port set in configuration for MediaSessionService");
		throw Glib::Thread::Exit();
	} else {
		port = config.mediaSessionServicePort;
	}

	shared_ptr<MediaSessionServiceHandler> handler(new MediaSessionServiceHandler());
	shared_ptr<TProcessor> processor(new MediaSessionServiceProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory ());

	shared_ptr<ThreadManager> threadManager= ThreadManager::newSimpleThreadManager(15);

	threadManager->threadFactory(threadFactory);

	shared_ptr<TThreadedServer> server(new TThreadedServer(processor,
					serverTransport, transportFactory,
					protocolFactory, threadFactory));

	i("Starting MediaSessionService");
	server->serve();

	i("MediaSessionService stopped finishing thread");
	throw Glib::Thread::Exit();
}

static void create_network_connection_service() {
	int port;

	if (!config.__isset.networkConnectionServicePort) {
		w("No port set in configuration for NetworkConnectionService");
		throw Glib::Thread::Exit();
	} else {
		port = config.networkConnectionServicePort;
	}

	shared_ptr<NetworkConnectionServiceHandler> handler(new NetworkConnectionServiceHandler());
	shared_ptr<TProcessor> processor(new NetworkConnectionServiceProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory ());

	shared_ptr<ThreadManager> threadManager= ThreadManager::newSimpleThreadManager(15);

	threadManager->threadFactory(threadFactory);

	shared_ptr<TThreadedServer> server(new TThreadedServer(processor,
					serverTransport, transportFactory,
					protocolFactory, threadFactory));

	i("Starting NetworkConnectionService");
	server->serve();

	i("NetworkConnectionService stopped finishing thread");
	throw Glib::Thread::Exit();
}

static void create_mixer_service() {
	int port;

	if (!config.__isset.mixerServicePort) {
		w("No port set in configuration for MixerService");
		throw Glib::Thread::Exit();
	} else {
		port = config.mixerServicePort;
	}

	shared_ptr<MixerServiceHandler> handler(new MixerServiceHandler());
	shared_ptr<TProcessor> processor(new MixerServiceProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory ());

	shared_ptr<ThreadManager> threadManager= ThreadManager::newSimpleThreadManager(15);

	threadManager->threadFactory(threadFactory);

	shared_ptr<TThreadedServer> server(new TThreadedServer(processor,
					serverTransport, transportFactory,
					protocolFactory, threadFactory));

	i("Starting MixerService");
	server->serve();

	i("MixerService stopped finishing thread");
	throw Glib::Thread::Exit();
}

int main(int argc, char **argv) {

	Glib::thread_init();

	config.__set_address(SERVER_ADDRESS);
	config.__set_serverServicePort(SERVER_SERVICE_PORT);
	config.__set_mediaSessionServicePort(SESSION_SERVICE_PORT);
	config.__set_networkConnectionServicePort(NETWORK_CONNECTION_SERVICE_PORT);
	config.__set_mixerServicePort(MIXER_SERVICE_PORT);

	sigc::slot<void> ss = sigc::ptr_fun(&create_server_service);
	Glib::Thread *serverServiceThread = Glib::Thread::create(ss, true);

	sigc::slot<void> mss = sigc::ptr_fun(&create_session_service);
	Glib::Thread::create(mss, true);

	sigc::slot<void> ncss = sigc::ptr_fun(&create_network_connection_service);
	Glib::Thread::create(ncss, true);

	sigc::slot<void> mxss = sigc::ptr_fun(&create_mixer_service);
	Glib::Thread::create(mxss, true);

	Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create(true);
	loop->run();
	serverServiceThread->join();

	// TODO: Finish all other threads and notify error

	return 0;
}
