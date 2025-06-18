server_client : examples/beej_stream_client.c server.c
	gcc examples/beej_stream_client.c -o targets/stream_client_ex
	gcc server.c client_handling.c -o targets/epoll_server