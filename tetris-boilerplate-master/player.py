from board import Direction, Rotation, Action

weights = [-0.3359425039858996, 0.00037326944887322176, -0.23889244727886194, -0.6942811749041925, -0.5897657292196904,
           -0.014930777954928871]


def find_total_blocks(board):
    counter = 0
    for y in range(24):
        for x in range(10):
            if (x, y) in board.cells:
                counter += 1
    return counter


# Ideas for what features to check were adapted from the Pierre Dellacherie algorithm

def find_landing_height(prev_board, board):
    piece_array = []
    for y in range(24):
        for x in range(10):
            if (x, y) not in prev_board.cells and (x, y) in board.cells:
                piece_array.append(24 - y)
    try:
        min_piece_y = min(piece_array)
    except:
        return 0
    max_piece_y = max(piece_array)
    piece_height = max_piece_y - min_piece_y + 1
    landing_height = min_piece_y + (piece_height / 2)
    return landing_height


def find_score_potential(board, prev_total_blocks):
    difference = abs(find_total_blocks(board) - prev_total_blocks)
    if (difference - 6) % 10 == 0:
        rows_cleared = ((difference - 6) / 10) + 1
        score_potential = 25 * (4 ** (rows_cleared - 1))
        return score_potential
    else:
        return 0


def find_row_transition(board):
    row_transitions = 0
    for y in range(24):
        for x in range(10):
            if (((x, y) in board.cells and (x + 1, y) not in board.cells) or (
                    (x, y) not in board.cells and (x + 1, y) in board.cells)):
                row_transitions += 1

    return row_transitions


def find_col_transition(board):
    col_transitions = 0
    for x in range(10):
        for y in range(24):
            if y < 23 and (((x, y) in board.cells and (x, y + 1) not in board.cells) or (
                    (x, y) not in board.cells and (x, y + 1) in board.cells)):
                col_transitions += 1

    return col_transitions


def find_holes(board):
    holes = 0
    for x in range(10):
        for y in range(23, -1, -1):
            if (x, y) not in board.cells and (x, y - 1) in board.cells:
                holes += 1
    return holes


def find_well_sums(board):
    heights_list = []
    wells_sums = 0
    for x in range(10):
        for y in range(24):
            if (x, y) in board.cells:
                heights_list.append(y)
                break
        else:
            heights_list.append(24)

    for x in range(1, 9):
        col_well_sum = 0
        for y in range(heights_list[x] - 1, -1, -1):
            if (x - 1, y) in board.cells and (x + 1, y) in board.cells:
                col_well_sum += 1
        wells_sums += ((col_well_sum + 1) * (col_well_sum / 2))

    col_well_sum = 0
    for y in range(heights_list[0] - 1, -1, -1):
        if (1, y) in board.cells:
            col_well_sum += 1
    wells_sums += ((col_well_sum + 1) * (col_well_sum / 2))

    col_well_sum = 0
    for y in range(heights_list[9] - 1, -1, -1):
        if (8, y) in board.cells:
            col_well_sum += 1
    wells_sums += ((col_well_sum + 1) * (col_well_sum / 2))

    return wells_sums


def score_board(prev_board, board):
    global weights
    score = 0
    prev_total_blocks = find_total_blocks(prev_board)
    parameters = [find_landing_height(prev_board, board), find_score_potential(board, prev_total_blocks),
                  find_row_transition(board), find_col_transition(board), find_holes(board), find_well_sums(board)]
    for i in range(6):
        score += weights[i] * parameters[i]
    return score


def print_board(board):
    print("--------")
    for y in range(24):
        s = ""
        for x in range(10):
            if (x, y) in board.cells:
                s += "#"
            else:
                s += "."
        print(s, y)


class Player:
    def choose_action(self, board):
        raise NotImplementedError


class MyPlayer(Player):
    def choose_action(self, board):
        best_score = -1000000000
        min_holes_generated = 100000
        best_moves = []

        for x in range(10):
            for y in range(4):
                sandbox = board.clone()
                xpos = sandbox.falling.left
                moves = []

                for reps in range(y):
                    if sandbox.falling is not None:
                        sandbox.rotate(Rotation.Clockwise)
                        moves.append(Rotation.Clockwise)

                while xpos > x and sandbox.falling is not None:
                    xpos = sandbox.falling.left - 1
                    sandbox.move(Direction.Left)
                    moves.append(Direction.Left)

                while xpos < x and sandbox.falling is not None:
                    xpos = sandbox.falling.left + 1
                    sandbox.move(Direction.Right)
                    moves.append(Direction.Right)

                if sandbox.falling is not None:
                    sandbox.move(Direction.Drop)
                    moves.append(Direction.Drop)

                penis = sandbox.clone()
                print(sandbox)

                for x1 in range(10):
                    for y1 in range(4):
                        sandbox = penis
                        xpos1 = sandbox.falling.left
                        moves1 = []

                        for reps1 in range(y1):
                            if sandbox.falling is not None:
                                sandbox.rotate(Rotation.Clockwise)
                                moves1.append(Rotation.Clockwise)

                        while xpos1 > x1 and sandbox.falling is not None:
                            xpos1 = sandbox.falling.left - 1
                            sandbox.move(Direction.Left)
                            moves1.append(Direction.Left)

                        while xpos1 < x1 and sandbox.falling is not None:
                            xpos1 = sandbox.falling.left + 1
                            sandbox.move(Direction.Right)
                            moves1.append(Direction.Right)

                        if sandbox.falling is not None:
                            sandbox.move(Direction.Drop)
                            moves1.append(Direction.Drop)

                        print_board(sandbox)
                        min_holes_generated = min(min_holes_generated, (find_holes(sandbox) - find_holes(board)))
                        score = score_board(board, sandbox)
                        if score > best_score:
                            best_score = score
                            temp = []
                            temp.extend(moves)
                            temp.extend(moves1)
                            best_moves = temp.copy()

        if min_holes_generated > 0:
            if board.discards_remaining > 0:
                return Action.Discard

        # assert False
        return best_moves


SelectedPlayer = MyPlayer
