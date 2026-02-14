#The purpose of this file is to build the board state. This will be a data structure that gives our program a clear view of what
#the board looks like at the current moment. To do this, it is passed a FEN string, and will parse that string into a matrix of 
#cells organized into ranks and files. Each of these cells will also keep track of any pieces on them.
import pygame
import math
import ChessPieces
#offset 167
class ChessBoard():
    def __init__(self,offset:tuple,cellsize:int,xMax:int,yMax:int,fenStr:str):
        self.offset = offset
        self.cellSize = cellsize
        self.xMax = xMax
        self.yMax = yMax
        self.colors = ["DarkSeaGreen","DarkGreen"]
        self.pieceParser = {
        "p":ChessPieces.Pawn,
        "r":ChessPieces.Rook,
        "n":ChessPieces.Knight,
        "b":ChessPieces.Bishop,
        "q":ChessPieces.Queen,
        "k":ChessPieces.King
        }
        self.boardMap = {}
        self.parsedFen = self.parse_fen_string(fenStr)

    #modify our board map to include the pieces present in the FEN string
    def parse_fen_board_state(self,fenString:str):
        rowNum = 0
        colNum = 0
        for currentChar in fenString:
            if currentChar == "/":
                rowNum += 1
                colNum = 0
                continue
            #if we read a number, these are empty cells and we can skip over them.
            if currentChar.isnumeric():
                colNum += int(currentChar)
                continue
            pieceClass = self.pieceParser.get(currentChar.lower())
            pieceColor = "Black"
            #for now, we continue on a failed lookup just for testing. In the future, this will error out instead.
            if pieceClass == None:
                continue
            if currentChar.isupper():
                pieceColor = "White"
            boardCell = self.boardMap[rowNum][colNum]
            newPiece = pieceClass(pieceColor,boardCell,self)
            boardCell[2] = newPiece
            colNum += 1
        return self.boardMap

    #builds a dictionary that holds every square on our board in row : [cell1, cell2,...] format. data looks like:
    #0 : [[Square1,Square1Color,PiecePresent],[Square2,Square2Color,PiecePresent]...]
    #1 : [[Square9,Square9Color,PiecePresent],[Square10,Square10Color,PiecePresent]...]
    #.
    #.
    #.
    def build_board_from_fen(self):
        self.boardMap = {}
        for yPos in range(0,self.yMax):
            currentFile = []
            for xPos in range(0,self.xMax):
                square = pygame.Rect((xPos*self.cellSize)+self.offset[0],(yPos*self.cellSize)+self.offset[1],self.cellSize,self.cellSize)
                tileColor = self.colors[(xPos+yPos)%2]
                currentFile.append([square,tileColor,None])
            self.boardMap[yPos] = currentFile
        #add the pieces from the FEN string onto the board map
        self.boardMap = self.parse_fen_board_state(self.parsedFen.get("BoardState"))
        return self.boardMap
    
    #this function will find the closest cell to a given position tuple(x,y), and return its entry in boardMap
    #pos is a position represented as a tuple (x,y), cellFlag tells us if we want to return the cell in board map or its row and file.
    def find_closest_cell(self,pos:tuple,cellFlag:bool):
        #we dont have to search ALL of the cells, we can find what row the click is in with floor((pos.y-board.offset(y))/board.cellSize)
        #we can find what file its in with floor((pos.x-board.offset(x))/board.cellSize)
        cellRow = math.floor((pos[1]-self.offset[1])/self.cellSize)
        cellFile = math.floor((pos[0]-self.offset[0])/self.cellSize)
        #if the index we find is greater than the size of our boardMap arrays or less than 0, the user clicked off the board
        if not cellFile in range(0,self.xMax):
            return None
        #same logic for cell rows, except with y instead of x.
        if not cellRow in range(0,self.yMax):
            return None
        if cellFlag:
            cloestCell = self.boardMap[cellRow][cellFile]
            return cloestCell
        else:
            return [cellFile,cellRow]
    

    #fen string are stored as BoardState ActiveColor CastlingRights EnPassantTargets HalfMove FullMove
    def parse_fen_string(self,fen:str):
        fenArray:list = fen.split()
        #fill missing fields with an empty entry
        for i in range(0,6-len(fenArray)):
            fenArray.append("")
        fenDict = {
            "BoardState": fenArray[0],
            "ActiveColor" : fenArray[1],
            "CastlingRights": fenArray[2],
            "EnPassantTargets": fenArray[3],
            "HalfMove" : fenArray[4],
            "FullMove" : fenArray[5]
        }
        #rather than using traditional notation, this program uses "White" and "Black" for active color.
        if fenDict["ActiveColor"] == "w":
            fenDict["ActiveColor"] = "White"
        else:
            fenDict["ActiveColor"] = "Black"
        return fenDict
