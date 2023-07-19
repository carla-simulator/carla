import numpy as np
import torch
import torch.nn as nn
from PIL import Image
from torchvision import models, transforms



class Detector(nn.Module):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)

    def input_transform(self, image):
        transform = transforms.Compose(transforms.ToTensor(), 
                                      transforms.Normalize(
                                      mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]))
        return transform(image)