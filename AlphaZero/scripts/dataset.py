import pandas as pd
import os
import math
import time
import tqdm
import random

folder_path = "../datasets/"
filename = "tactic_evals.csv"
# A1 = 290.680623072
# A2 = 3.096181612
# B1 = 0.5
A1 = 111.714640912
A2 = 1.5620688421
B1 = 0.0

def main():
    
    file_path = os.path.join(folder_path, filename)
    df = pd.read_csv(file_path)

    collumns = ['fen', 'move', 'value']

    processed_data = {
        'fen' : [],
        'move': [],
        'value': []
        }

    for i in tqdm.tqdm(range(0, len(df))):
        
        player = df["FEN"][i].split(" ")[1] 
        centipawns = df['Evaluation'][i]
        action = df['Move'][i]

        if (not isinstance(action, str)) or (not isinstance(df["FEN"][i], str)) or (not isinstance(centipawns, str)):
            continue
        
        if len(df["FEN"][i]) == 0 or len(action) == 0 or len(centipawns) == 0:
            continue

        if centipawns[0] == '#':
            sign = 1 if centipawns[1] == '+' else -1

            value = 1 if player == 'w' else -1

            value *= sign

            # print(centipawns, " - ", player, " - ", value)

        elif centipawns[1:].isdigit():
            sign = 1 if centipawns[0] == '+' else -1
            centipawns = float(centipawns[1:])
            centipawns *= sign

            value = math.atan(centipawns / A1) / A2 + B1
            flip = 1 if player == 'w' else -1

            value *= flip

            # print(centipawns, " centipawns = ", value, " win prob ", player, ' - ', df["FEN"][i], ' - ', action)

        else:
            centipawns = float(centipawns)
            value = math.atan(centipawns / A1) / A2 + B1
        
        
        # # Check for pawn promotion
        # def expand_fen(fen_part):
        #     expanded = ""
        #     for char in fen_part:
        #         if char.isdigit():
        #             expanded += " " * int(char)
        #         else:
        #             expanded += char
        #     return expanded

        # board = expand_fen(df["FEN"][i].split(" ")[0])
        # start_col = ord(action[0]) - ord('a')
        # start_row = 8 - int(action[1])

        # piece = board.split("/")[start_row][start_col]

        # if (piece == 'P' and player == 'w' and action[1] == '7' and action[3] == '8') or \
        #    (piece == 'p' and player == 'b' and action[1] == '2' and action[3] == '1'):
        #     promotion = random.choice(['q', 'r', 'n', 'b'])
        #     action += promotion
        
        processed_data['fen'].append(df["FEN"][i])
        processed_data['move'].append(action)
        processed_data['value'].append(value)

    pd_processed = pd.DataFrame(processed_data)
    pd_processed.to_csv(f"{folder_path}processed_{filename}", index=False)

if __name__ == "__main__":
    main()