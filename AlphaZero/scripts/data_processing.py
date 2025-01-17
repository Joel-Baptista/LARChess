import pandas as pd
import chess

def load_csv(file_path):
    """Load a CSV file into a Pandas DataFrame."""
    try:
        df = pd.read_csv(file_path)
        print("CSV file loaded successfully.")
        return df
    except Exception as e:
        print(f"An error occurred while loading the CSV file: {e}")
        return None

def preprocess_data(df):
    """Preprocess the DataFrame."""
    # Example of preprocessing steps:
    # ndf = pd.DataFrame(columns=['board', 'move', 'player', 'winner'])

    ndf_dict = {
            'fen': [], 
            'move': [],
            'value': [],
            }

    for index, row in df.iterrows():
        board = chess.Board()
        moves = df.at[index, 'moves'].split(" ")
        print("Game: ", index)
        for move in moves:
            chess_move = board.parse_san(move)

            if board.turn == chess.WHITE:
                player = 'white'
            else:
                player = 'black'
            
            value = 1 if df.at[index, 'winner'] == player else -1

            ndf_dict['fen'].append(board.fen())
            ndf_dict['move'].append(chess_move)
            ndf_dict['value'].append(value)
            # ndf = ndf._append({'board': board.fen(), 'move': chess_move, 'player': player, 'winner': df.at[index, 'winner']}, ignore_index=True)

            board.push(chess_move)
            
    ndf = pd.DataFrame.from_dict(ndf_dict)
    
    # # 1. Rename columns
    # df.rename(columns={'old_column_name': 'new_column_name'}, inplace=True)
    
    # # 2. Drop unnecessary columns
    # df.drop(columns=['column_to_drop'], inplace=True)
    
    # # 3. Fill missing values
    # df.fillna({'column_with_nan': 0}, inplace=True)
    
    # # 4. Filter rows based on a condition
    # df = df[df['column_name'] > 0]
    
    # # 5. Create new columns or modify existing ones
    # df['new_column'] = df['existing_column'] * 2
    
    return ndf

def save_csv(df, output_path):
    """Save the DataFrame to a new CSV file."""
    try:
        df.to_csv(output_path, index=False)
        print(f"Data saved successfully to {output_path}")
    except Exception as e:
        print(f"An error occurred while saving the CSV file: {e}")

def main():
    # Set your file paths here
    input_file_path = '../datasets/games.csv'  # Replace with your input CSV file path
    output_file_path = '../datasets/games_processed_v2.csv'  # Replace with your desired output CSV file path
    
    # Load the CSV file
    df = load_csv(input_file_path)
    
    if df is not None:
        # Preprocess the data
        df = preprocess_data(df)
        
        # Save the processed data to a new CSV file
        save_csv(df, output_file_path)

if __name__ == "__main__":
    main()