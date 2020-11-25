rm main
g++ -std=c++11 -o main main.cpp vcoin.h vhasher.h -fopenmp $(pkg-config --cflags --libs libbitcoin)
