import pandas as pd
import numpy as np

n_samples = 100
np.random.seed(42)

data = {
    'current': np.concatenate([
        np.random.normal(0.1, 0.02, 33),  
        np.random.normal(1.5, 0.3, 33),   
        np.random.normal(3.5, 0.5, 34)    
    ]),
    'vibration': np.concatenate([
        np.random.normal(0.01, 0.005, 33), 
        np.random.normal(0.5, 0.1, 33),    
        np.random.normal(1.2, 0.2, 34)     
    ]),
    'hall_sensor': np.concatenate([
        np.zeros(33),                      
        np.random.uniform(50, 150, 33),    
        np.random.uniform(200, 400, 34)    
    ]),
    'stage': np.concatenate([
        ['Idle']*33, 
        ['Low_Speed']*33, 
        ['High_Speed']*34
    ])
}

df = pd.DataFrame(data)
df = df.sample(frac=1).reset_index(drop=True)

df.to_csv('sewing_data.csv', index=False)
print("Create file sewing_data.csv success!")