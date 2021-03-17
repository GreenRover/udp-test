all: clean client_IPerf client_TibRv client_TibRv_fixed server

client_IPerf: client_IPerf.c common.h
	$(CC) -O3 $< -o $@

client_TibRv: client_TibRv.c common.h
	$(CC) -O3 $< -o $@

client_TibRv_fixed: client_TibRv_fixed.c common.h
	$(CC) -O3 $< -o $@

server: server.c common.h
	$(CC) -O3 $< -o $@

.PHONY: clean

clean:
	rm -f client_IPerf client_TibRv client_TibRv_fixed server