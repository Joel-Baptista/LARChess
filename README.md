# YACE
Yet Another Chess Engine (now in C++). 

This project marks my first venture into C++ and is a continuation of the original Python-based project from [AlphaZero_from_Scratch](https://github.com/Joel-Baptista/AlphaZero_from_Scratch). As noted earlier, the search space in Chess is too vast to efficiently navigate using a language as slow as Python. Therefore, we have decided to implement the chess engine and the AlphaZero algorithm in C++, which is the primary objective of this repository.

The outcomes of this project will later be utilized in the [RoboChess](https://github.com/Joel-Baptista/YACE) project.

The current implementation uses 2D arrays, which is a straightforward approach but not the most efficient. While 2D arrays are easier to code, utilizing bitboards would significantly improve performance and efficiency.