# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
Pygame Display Module

This module provides a threaded Pygame display for showing camera images from CARLA.
It runs in a separate thread to avoid blocking the main application thread and
provides a simple interface for updating the displayed image.
"""
# Hide pygame welcome message
import os
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = "hide"
import pygame
import numpy as np
import threading
import queue
import time
from typing import Tuple, Optional, List

class PygameDisplay:
    """
    A class to display images using Pygame in a separate thread.
    
    This class creates a Pygame window in a background thread and provides
    methods to update the displayed image without blocking the main thread.
    It handles window creation/destruction and automatically adjusts the window
    size to match the first image received.
    """
    def __init__(self, window_title: str = "NUREC Camera View", cell_width: int = 481, cell_height: int = 271) -> None:
        """
        Initialize the pygame display thread.
        
        Args:
            window_title (str): Title of the pygame window
        """
        self.window_title = window_title
        self.image_queue: queue.Queue[Tuple[np.ndarray, Tuple[int, int], Tuple[int, int]]] = queue.Queue(maxsize=10)
        self.initialized = False
        self.cell_width = cell_width
        self.cell_height = cell_height
        self.running = True
        # Start the display thread immediately
        self.display_thread = threading.Thread(target=self._display_loop)
        self.display_thread.daemon = True  # Thread will exit when main program exits
        self.display_thread.start()
        self.surfaces: List[Optional[pygame.Surface]] = []
    
    def setImage(self, image: np.ndarray, dims: Tuple[int, int] = (1,1), pose: Tuple[int, int] = (0, 0)) -> None:
        """
        Update the displayed image.
        
        This method places the new image in the queue for the display thread to process.
        If the queue is full (previous image hasn't been processed yet), the oldest
        image is discarded.
        
        Args:
            image (numpy.ndarray): Numpy array representing the image (HxWx3 RGB format)
            width (int): Width of the images (count)
            pose (numpy.ndarray): Pose of the image
        """
        self.image_queue.put((image, dims, pose))
    
    def _display_loop(self) -> None:
        """
        Main loop for the pygame display thread.
        
        This method runs in a separate thread and continuously checks for new images
        to display while handling Pygame events. It initializes the Pygame window
        when the first image is received and handles clean shutdown when the window
        is closed.
        """
        screen = None
        
        try:
            # Initialize pygame
            pygame.init()
            
            # Wait for the first image to set window size
            first_image, dims, pose = self.image_queue.get()
            # Initialize surfaces list with proper typing
            self.surfaces = [None for _ in range(dims[0] * dims[1])]
            
            # Create the window with fixed cell sizes
            window_width = self.cell_width * dims[0]
            window_height = self.cell_height * dims[1]
            if self.running:
                screen = pygame.display.set_mode((window_width, window_height))
            if self.running:
                pygame.display.set_caption(self.window_title)
            self.initialized = True
            
            # requeue first image
            self.image_queue.put((first_image, dims, pose))
            
            # Main display loop
            while self.running:
                # Handle pygame events
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        self.running = False
                        break
                
                # Check for new images
                try:
                    image, dims, pose = self.image_queue.get(timeout=0.1)
                    self._display_image(screen, image, dims, pose, self.cell_width, self.cell_height)
                except queue.Empty:
                    # No new image, continue waiting
                    pass
                
                # Small delay to prevent high CPU usage
                time.sleep(0.01)
        
        except Exception as e:
            print(f"Error in pygame display thread: {e}")
        
        finally:
            pygame.quit()
    
    def _display_image(self, screen: pygame.Surface, image: np.ndarray, dims: Tuple[int, int], pose: Tuple[int, int], cell_width: int, cell_height: int) -> None:
        """
        Helper method to display an image on the screen, centered in a fixed-size cell.
        
        Args:
            screen (pygame.Surface): The pygame screen surface to draw on
            image (numpy.ndarray): Image data as numpy array (HxWx3 RGB format)
            dims (tuple): Grid dimensions (rows, cols)
            pose (tuple): Position in grid (row, col)
            cell_width (int): Fixed width of each cell
            cell_height (int): Fixed height of each cell
        """
        # Convert image to pygame surface
        # Transpose to match pygame's coordinate system
        image_rgb = np.transpose(image, (1, 0, 2))
        surface = pygame.surfarray.make_surface(image_rgb)
        self.surfaces[pose[0]*dims[1]+pose[1]] = surface

        # Clear the screen
        screen.fill((0, 0, 0))

        for i in range(dims[0]):
            for j in range(dims[1]):
                current_surface = self.surfaces[i*dims[1]+j]
                if current_surface is not None:
                    # Calculate cell position
                    cell_x = i * cell_width
                    cell_y = j * cell_height
                    
                    # Get image dimensions
                    img_width = current_surface.get_width()
                    img_height = current_surface.get_height()
                    
                    # Calculate centering offset
                    offset_x = (cell_width - img_width) // 2
                    offset_y = (cell_height - img_height) // 2
                    
                    # Draw the image centered in the cell
                    screen.blit(current_surface, (cell_x + offset_x, cell_y + offset_y))
        
        pygame.display.flip()

    def destroy(self) -> None:
        """
        Destroy the pygame display.
        """
        self.running = False
        pygame.event.post(pygame.event.Event(pygame.QUIT))
        self.display_thread.join()