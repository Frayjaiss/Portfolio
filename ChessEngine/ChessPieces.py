#this file is responsible for holding all of the chess piece logic. It will define various classes that will tell our program
#how pieces can move. It is not concerned with what a piece can do in a given position, just how a piece can move in a vacuum. 

#first we define some movement categories: sliding, stepping, and hopping. Sliding pieces can slide along multiple positions in
#a single move (rook, bishop, queen), stepping pieces move one tile in a direction (king, pawn), and hopping pieces jump appear 
#at a given position following a pattern (knight).

#directions is an array containing all the valid directions the piece can move. eg: directions = ["up","down","left","right"] is a rook
#and directions = ["uleft","uright","dleft","dright"] is a bishop

import pygame

class Piece():
    def __init__(self,color,name):
        self.color = color
        imgString = color+name
        imgPath = "Assets/"+imgString+".png"
        self.asset = pygame.image.load(imgPath).convert_alpha()

class SlidingPiece(): 
    def __init__(self,directions):
        self.directions = directions

class SteppingPiece():
    def __init__(self,directions):
        self.directions = directions

#Hopping pieces difer from stepping and sliding in the they take a MovementPattern class that defines how the move on the board.
class HoppingPiece():
    def __init__(self,directions):
        self.directions = directions


class Pawn(Piece):
    def __init__(self,color):
        super().__init__(color,self.__class__.__name__)
        self.movement = SteppingPiece(["up"])

class Rook(Piece):
    def __init__(self, color):
        super().__init__(color, self.__class__.__name__)
        self.movement = SlidingPiece(["up","down","left","right"])

class Bishop(Piece):
    def __init__(self, color):
        super().__init__(color, self.__class__.__name__)
        self.movement = SlidingPiece(["uleft","uright","dleft","dright"])   

class Queen(Piece):
    def __init__(self, color):
        super().__init__(color, self.__class__.__name__)
        self.movement = SlidingPiece(["up","down","left","right","uleft","uright","dleft","dright"])     

class King(Piece):
    def __init__(self, color):
        super().__init__(color, self.__class__.__name__)
        self.movement = SteppingPiece(["up","down","left","right","uleft","uright","dleft","dright"])

class Knight(Piece):
    def __init__(self, color):
        super().__init__(color, self.__class__.__name__)
        self.movement = HoppingPiece(None)      