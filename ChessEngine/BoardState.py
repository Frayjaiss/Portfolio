#The purpose of this file is to build the board state. This will be a data structure that gives our program a clear view of what
#the board looks like at the current moment. To do this, it is passed a FEN string, and will parse that string into a matrix of 
#cells organized into ranks and files. Each of these cells will also keep track of any pieces on them.
import pygame
import ChessPieces


pieceParser = {
    "p":ChessPieces.Pawn,
    "r":ChessPieces.Rook,
    "n":ChessPieces.Knight,
    "b":ChessPieces.Bishop,
    "q":ChessPieces.Queen,
    "k":ChessPieces.King
}

#modify our board map to include the pieces present in the FEN string
def parse_fen_strin(boardMap,fenString:str):
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
        pieceClass = pieceParser.get(currentChar.lower())
        pieceColor = "Black"
        #for now, we continue on a failed lookup just for testing. In the future, this will error out instead.
        if pieceClass == None:
            continue
        if currentChar.isupper():
            pieceColor = "White"
        newPiece = pieceClass(pieceColor)
        boardCell = boardMap[rowNum][colNum]
        boardCell[2] = newPiece
        colNum += 1
    return boardMap

#builds a dictionary that holds every square on our board in row : [cell1, cell2,...] format. data looks like:
#0 : [[Square1,Square1Color,PiecePresent],[Square2,Square2Color,PiecePresent]...]
#1 : [[Square9,Square9Color,PiecePresent],[Square10,Square10Color,PiecePresent]...]
#.
#.
#.
def build_board_from_fen(fenString):
    boardMap = {}
    #(x,y) offset for board
    boardOffset = (167,0)
    xMax: int = 8
    yMax: int = 8
    cellSize: int = 60
    colors = ["DarkSeaGreen","DarkGreen"]
    for yPos in range(0,yMax):
        currentFile = []
        for xPos in range(0,xMax):
            square = pygame.Rect((xPos*cellSize)+boardOffset[0],(yPos*cellSize)+boardOffset[1],cellSize,cellSize)
            tileColor = colors[(xPos+yPos)%2]
            currentFile.append([square,tileColor,None])
        boardMap[yPos] = currentFile
    #add the pieces from the FEN string onto the board map
    boardMap = parse_fen_strin(boardMap,fenString)
    return boardMap