all:
	@echo "Compile..."
	g++ main.cpp -o XLander -lX11 -w
	@echo "Run..."
	./XLander
