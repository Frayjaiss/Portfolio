#this file is responsible for holding all of the chess piece logic. It will define various classes that will tell our program
#how pieces can move. It is not concerned with what a piece can do in a given position, just how a piece can move in a vacuum. 

#first we define some movement categories: sliding, stepping, and hopping. Sliding pieces can slide along multiple positions in
#a single move (rook, bishop, queen), stepping pieces move one tile in a direction (king, pawn), and hopping pieces jump appear 
#at a given position following a pattern (knight).

#directions is an array containing all the valid directions the piece can move. eg: directions = ["up","down","left","right"] is a rook
#and directions = ["uleft","uright","dleft","dright"] is a bishop

import pygame

################################# Logic Classes ###########################################

class Piece():
    def __init__(self,color,name,startingCell,board,stepLimit):
        self.color = color
        imgString = color+name
        imgPath = "Assets/"+imgString+".png"
        self.asset = pygame.image.load(imgPath).convert_alpha()
        self.cell = startingCell
        self.board = board
        self.stepLimit = stepLimit
    
    def set_occupying_cell(self,cell):
        self.cell = cell

    def piece_clicked(self):
        validMoves:list = self.movement.calc_moveable_cells(self.board,self,self.stepLimit)
        return validMoves

    def move_piece(self,newCell):
        self.cell[2] = None
        newCell[2] = self
        self.set_occupying_cell(newCell)

#movement for a piece that slides along in a straight line until it reaches another piece or the edge of the board. Stepping pieces 
#are just sliding pieces with a limited amount of steps.
class SlidingPiece(): 
    def __init__(self,directions):
        self.directions = directions

    #recursive function that finds all connected cells a sliding piece can slide along.
    def find_next_cell(self,coordinate:tuple,dir:tuple,board,color,stepLimit,cellArray = None):
        if cellArray == None:
            cellArray = []
        #we only want to flip the direction on black pieces on the first call to find_next_cell
        if color == "Black" and cellArray == []:
            dx,dy = dir
            dir = (-dx,-dy)
        #Ycoord in matrix
        file = coordinate[0]+dir[0]
        #Xcoord in matrix
        rank = coordinate[1]+dir[1]
        #reached the end of the board, stop recursion
        if (not rank in range(0,board.yMax)) or (not file in range(0,board.xMax)):
            return cellArray
        #if the stepLimit is at 0, we can break recursion as well
        if stepLimit == 0:
            return cellArray
        validCell = board.boardMap[rank][file]
        #cell is occupied by a friendly piece, stop recursion.
        if validCell[2] != None:
            #if the piece is an enemy piece, add that cell to the valid moves before ending recursion.
            if validCell[2].color != color:
                cellArray.append(validCell)
            return cellArray
        cellArray.append(validCell)
        return self.find_next_cell((file,rank),dir,board,color,stepLimit-1,cellArray)
    

    #this function returns all the possible cells this piece can move to. This list will be further pruned by the engine for legal moves only
    #stepping piece only move 1 square at a time, so grab the immediate cells around them.
    def calc_moveable_cells(self,board,piece,stepLimit:int):
        boardMap = board.boardMap
        #cell[0] is the rect of the current cell.
        currentPos:tuple = (piece.cell[0].x,piece.cell[0].y)
        currentRowAndFile:list = board.find_closest_cell(currentPos,False)
        moveableCellIndexes = []
        for dir in self.directions:
            foundCells = []
            match dir:
                case "up":
                    foundCells = self.find_next_cell(currentRowAndFile,(0,-1),board,piece.color,stepLimit)
                case "down":
                    foundCells = self.find_next_cell(currentRowAndFile,(0,1),board,piece.color,stepLimit)
                case "left":
                    foundCells = self.find_next_cell(currentRowAndFile,(-1,0),board,piece.color,stepLimit)
                case "right":
                    foundCells = self.find_next_cell(currentRowAndFile,(1,0),board,piece.color,stepLimit)
                case "uleft":
                    foundCells = self.find_next_cell(currentRowAndFile,(-1,-1),board,piece.color,stepLimit)
                case "uright":
                    foundCells = self.find_next_cell(currentRowAndFile,(1,-1),board,piece.color,stepLimit)
                case "dleft":
                    foundCells = self.find_next_cell(currentRowAndFile,(-1,1),board,piece.color,stepLimit)
                case "dright":
                    foundCells = self.find_next_cell(currentRowAndFile,(1,1),board,piece.color,stepLimit)
                case _:
                    continue
            moveableCellIndexes.extend(foundCells)
        return moveableCellIndexes


#Hopping pieces difer from stepping and sliding in the they take a MovementPattern class that defines how the move on the board.
class HoppingPiece():
    def __init__(self,directions):
        self.directions = directions
    
    def calc_moveable_cells(self,board,piece,stepLimit):
        return []


################################# Chess pieces #############################################

class Pawn(Piece):
    def __init__(self,color,startingCell,board):
        #Pawns start with a step limit of 2, reduced to 1 after their first move.
        stepLimit = 2
        super().__init__(color, self.__class__.__name__,startingCell,board,stepLimit)
        self.movement = SlidingPiece(["up"])
        self.firstMove = True
    #extend Piece.move_piece
    def move_piece(self,newCell):
        super().move_piece(newCell)
        if self.firstMove:
            self.firstMove = False
            self.stepLimit = 1
    
    #pawns have very unique movement rules in that theyre the only piece that captures differently then they move.
    #to implement this, were going to prune the validMoves list of any cells that contain a piece, and then make another validMoves
    #list that checks the diagonal squares for enemy pieces. We will return these two lists combined.
    def piece_clicked(self):
        validMoves:list = []
        #get the valid moves
        Moves:list = self.movement.calc_moveable_cells(self.board,self,self.stepLimit)
        #prune the moves, only keeping the ones that do not move the pawn onto a piece.
        for moveCell in Moves:
            if moveCell[2] == None:
                validMoves.append(moveCell)
        #check for captures now by chaning our movement directions to up diagonals and pruning for squares only with enemy pieces on them
        self.movement.directions = ["uleft","uright"]
        validCaptures:list = self.movement.calc_moveable_cells(self.board,self,1)
        for moveCell in validCaptures:
            if moveCell[2] != None:
                if moveCell[2].color != self.color:
                    validMoves.append(moveCell)
        #reset our valid directions for next time
        self.movement.directions = ["up"]
        return validMoves

class Rook(Piece):
    def __init__(self, color,startingCell,board):
        stepLimit = max(board.xMax,board.yMax)
        super().__init__(color, self.__class__.__name__,startingCell,board,stepLimit)
        self.movement = SlidingPiece(["up","down","left","right"])

class Bishop(Piece):
    def __init__(self, color,startingCell,board):
        stepLimit = max(board.xMax,board.yMax)
        super().__init__(color, self.__class__.__name__,startingCell,board,stepLimit)
        self.movement = SlidingPiece(["uleft","uright","dleft","dright"])   

class Queen(Piece):
    def __init__(self, color,startingCell,board):
        stepLimit = max(board.xMax,board.yMax)
        super().__init__(color, self.__class__.__name__,startingCell,board,stepLimit)
        self.movement = SlidingPiece(["up","down","left","right","uleft","uright","dleft","dright"])     

class King(Piece):
    def __init__(self, color,startingCell,board):
        stepLimit = 1
        super().__init__(color, self.__class__.__name__,startingCell,board,stepLimit)
        self.movement = SlidingPiece(["up","down","left","right","uleft","uright","dleft","dright"])

class Knight(Piece):
    def __init__(self, color,startingCell,board):
        super().__init__(color, self.__class__.__name__,startingCell,board,0)
        self.movement = HoppingPiece([(0,-1),(0,-1),(-1,0)])      