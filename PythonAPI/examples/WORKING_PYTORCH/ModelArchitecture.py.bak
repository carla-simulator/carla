import torch 
import torch.nn as nn
import torch.nn.functional as F
import torchvision

class QNetwork(nn.Module):
    def __init__(self, my_pretrained_model, seed):
        super(QNetwork, self).__init__()
        self.seed = torch.manual_seed(seed)
        self.pretrained = my_pretrained_model
        self.fc = nn.Sequential(nn.Linear(1000, 512), #changed 2048,512 to 1000,512
                                 nn.ReLU(),
                                 nn.Dropout(0.2),
                                 nn.Linear(512, 4),
                                 nn.LogSoftmax(dim=1))
    def forward(self, x):
        x = self.pretrained(x)
        x = self.fc(x)
        return x