all:
	@echo "Compile..."
	g++ main.cpp -o XLander -lX11
	@echo "Run..."
	./XLander
