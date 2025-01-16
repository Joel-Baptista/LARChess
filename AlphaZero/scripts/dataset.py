import pandas as pd
import os
import math
import time
import tqdm

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
        
        
        processed_data['fen'].append(df["FEN"][i])
        processed_data['move'].append(action)
        processed_data['value'].append(value)

    pd_processed = pd.DataFrame(processed_data)
    pd_processed.to_csv(f"{folder_path}processed_{filename}")

if __name__ == "__main__":
    main()