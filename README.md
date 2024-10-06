# LARChess

LARChess (Laboratory of Automation and Robotics Chess) is a chess engine built and trained in the [Laboratory of Automation and Robotics](http://lars.mec.ua.pt/)

This project represents my first venture into C++ development and is a continuation of the original Python-based project from [AlphaZero_from_Scratch](https://github.com/Joel-Baptista/AlphaZero_from_Scratch). As previously noted, the search space in chess is vast, making Python too slow for efficient navigation. To address this, we've decided to implement the chess engine and the AlphaZero algorithm in C++, which is the primary focus of this repository.

The outcomes of this project will eventually be integrated into the [RobotDeepChess](https://github.com/Joel-Baptista/RobotDeepChess) project.

## Requirements

This project is largely self-contained, with most dependencies included in the respective packages. The only external dependencies are [LibTorch](https://pytorch.org/get-started/locally/) and[OpenGL 4.0](https://www.opengl.org/). We are currently using the version of **LibTorch** compatible with **CUDA 11.8**. For convenience, we've also created a Docker image based on **nvidia/cuda:11.8.0-devel-ubuntu22.04**.

## Packages

This project is divided into several packages, each functioning as a standalone case study. However, some packages have dependencies on others.

- **ChessEngine**: A basic implementation of a chess engine with minimal preprocessed logic, where most computations are done on the fly. Although slower, it is simpler to understand.
- **ChessGUI**: A graphical user interface for the chess engine, developed using [OpenGL](https://www.opengl.org/), heavily inspired by this [Youtube tutorial](https://www.youtube.com/watch?v=W3gAzLwfIP0&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&ab_channel=TheCherno)l. While some practices may be outdated, they are sufficient for this simple interface.
- **ChessGame**: This package integrates the previous two to create a playable game. You can play against a basic bot that employs an Alpha-Beta pruning approach.
- **BBChessEngine**: This package replaces **ChessEngine** by implementing a more advanced strategy commonly used in chess engines, utilizing bitboards and pre-calculated attack tables to maximize computation speed. The code in this package is heavily influenced by this [Youtube tutorial](https://www.youtube.com/watch?v=QUNP-UjujBM&list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs&ab_channel=ChessProgramming). (Note: If you are considering using this implementation, please also consider [Stockfish](https://github.com/official-stockfish/Stockfish)).
- **BBChessGame**: This package combines **BBChessEngine** and **ChessGUI** to perform the same functions as **ChessGame**, but with enhanced speed. The result is a moderately strong chess engine that you can play against.
- **AlphaZero**: This package is the core objective of the project. It leverages **BBChessEngine** to implement the AlphaZero Algorithm, building on my previous Python implementation in [AlphaZero_from_Scratch](https://github.com/Joel-Baptista/AlphaZero_from_Scratch). This is still a work in progress.
