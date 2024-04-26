
all:
	@make build
	@make run


build:
	@cls
	@echo Started: %date% %time%
	g++ -std=c++17 main.cpp -I"include" -L"lib" -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -o main
	@echo Complete build: %date% %time%

run:
	./main