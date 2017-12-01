import numpy as np
import torch

a = torch.ones(5)
b = torch.from_numpy(np.ones(5, dtype=np.float32))
print(a)
print(b)
assert torch.sum(a==b) == 5
