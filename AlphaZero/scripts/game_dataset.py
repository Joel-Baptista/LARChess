import chess
import pandas as pd
from pathlib import Path

def detect_en_passant_square(board, move_obj):
    """
    Detect en passant square if the move is a double pawn push.
    """
    if board.piece_type_at(move_obj.from_square) == chess.PAWN:
        from_rank = chess.square_rank(move_obj.from_square)
        to_rank = chess.square_rank(move_obj.to_square)

        # Double push by white from rank 1 to 3, or black from 6 to 4
        if abs(to_rank - from_rank) == 2:
            # The square "behind" the pawn is the en passant target
            file = chess.square_file(move_obj.from_square)
            rank = (from_rank + to_rank) // 2
            return chess.square_name(chess.square(file, rank))
    return "-"  # No en passant square


def create_game_dataset(filename=None):
    """
    Create a dataset from a sequence of chess moves in standard algebraic notation (SAN).
    Each entry in the dataset contains the current position, the move made, and the next position.
    """
       
    if filename:
        df = pd.read_csv(filename)
        move_sequence = df['moves'].dropna().tolist()

    
    data = []
    for idx, move_list in enumerate(move_sequence):
        print(f"Processing game {idx + 1}/{len(move_sequence)}...")
            
        # Initialize board and list for data
        board = chess.Board()
        move_list = move_list.split()
        
        # Iterate through the moves
        for move in move_list:
            try:
                current_fen = board.fen()  # FEN before move
                move_obj = board.parse_san(move)  # parse but don't push yet
                uci_move = move_obj.uci()

                en_passant_square = detect_en_passant_square(board, move_obj)

                # Predict the en passant square
                board.push(move_obj)
                
                next_fen_parts = board.fen().split(" ")
                next_fen_parts[3] = en_passant_square  # Replace the en passant field
                fixed_next_fen = " ".join(next_fen_parts)
                next_fen = board.fen()
                data.append({
                    "current_position": current_fen,
                    "move": uci_move,
                    "next_position": fixed_next_fen
                })
            except ValueError as e:
                print(f"Error processing move '{move}': {e}")
                continue

    # Create DataFrame
    df = pd.DataFrame(data)

    # Display or save the DataFrame
    return df

if __name__ == "__main__":
    # Save to CSV if needed
    home_path = Path.home()
    print(f"Home path: {home_path}")
    df = create_game_dataset(f"{home_path}/projects/LARChess/AlphaZero/datasets/games.csv")
    df.to_csv("game_dataset.csv", index=False)
    print("Game dataset saved to 'game_dataset.csv'.")