import sys
import itertools
import time
import copy

TURN_SIMULATED = 4

class Hurdle:
	def __init__(self, position, stunDuration, field):
		self.position = position
		self.starting_position = position
		self.stunDuration = stunDuration
		self.field = field
		self.finished = False if self.position < 29 else True
		self.turnFinished = -1 if self.finished else 0
		self.optimalTurnFinished, self.optimalDistance = self.find_optimal(TURN_SIMULATED)

	def playTurn(self, action, turn_nb):
		if self.position >= 29 or self.field == "GAME_OVER":
			return
		if self.stunDuration > 0:
			self.stunDuration -= 1
			return
		d = {"LEFT": 1, "RIGHT": 3, "DOWN": 2}
		if action == "UP":
			self.position += 2
			if self.position < 30 and self.field[self.position] == "#":
				self.stunDuration = 3
		else:
			for _ in range(d[action]):
				self.position += 1
				if self.position < 30 and self.field[self.position] == "#":
					self.stunDuration = 3
					break
		if self.position >= 29:
			self.finished = True
			self.turnFinished = turn_nb
	
	def find_optimal(self, nb_turn):
		s = self.position
		if s >= 29 or self.field == "GAME_OVER":
			return (0, 29)
		for i in range(nb_turn):
			if s + 1 < 30 and self.field[s + 1] == "#":
				s += 2
			elif s + 2 < 30 and self.field[s + 2] == "#":
				s += 1
			elif s + 3 < 30 and self.field[s + 3] == "#":
				s += 2
			else:
				s += 3
			if s >= 29:
				return (i + 1, 29)
		return (-1, s)
			
	def evalutate(self):
		if self.field == "GAME_OVER":
			return 0
		if self.finished:
			return self.optimalTurnFinished / self.turnFinished * 100
		else:
			if self.optimalTurnFinished != -1:
				self.optimalDistance += (TURN_SIMULATED - self.optimalTurnFinished) * 2
		return (self.position - self.starting_position) / (self.optimalDistance - self.starting_position) * 100

class Archery:
	def __init__(self, coordinates, remainingTurns, windSpeed):
		self.coordinates = coordinates
		self.remainingTurns = remainingTurns
		self.windSpeed = windSpeed

	def playTurn(self, action):
		if self.remainingTurns == 0 or self.windSpeed == "GAME_OVER":
			return
		curWind = int(self.windSpeed[-self.remainingTurns])
		if action == "UP":
			self.coordinates[1] += curWind
		elif action == "DOWN":
			self.coordinates[1] -= curWind
		elif action == "LEFT":
			self.coordinates[0] -= curWind
		elif action == "RIGHT":
			self.coordinates[0] += curWind
		self.remainingTurns -= 1
		if self.coordinates[0] < -20:
			self.coordinates[0] = -20
		if self.coordinates[0] > 20:
			self.coordinates[0] = 20
		if self.coordinates[1] < -20:
			self.coordinates[1] = -20
		if self.coordinates[1] > 20:
			self.coordinates[1] = 20
	
	def evaluate(self):
		if self.windSpeed == "GAME_OVER":
			return 0
		else:
			return (40 - abs(self.coordinates[0]) - abs(self.coordinates[1])) / 40 * 100


class Skating:
	def __init__(self, position, risk, stunDuration, remainingTurns, curRisk):
		self.position = position
		self.starting_position = position
		self.risk = risk
		self.starting_risk = risk
		self.stunDuration = stunDuration
		self.remainingTurns = remainingTurns
		self.curRisk = curRisk
	
	def playTurn(self, action, turn_nb):
		if turn_nb > 0 or self.curRisk == "GAME_OVER":
			return
		if self.stunDuration > 0:
			self.stunDuration -= 1
			return
		d = {"LEFT": "L", "RIGHT": "R", "DOWN": "D", "UP": "U"}
		riskIdx = self.curRisk.index(d[action])
		if riskIdx == 0:
			self.position += 1
			if self.risk > 0:
				self.risk -= 1
		elif riskIdx == 1:
			self.position += 2
		elif riskIdx == 2:
			self.position += 2
			self.risk += 1
		else:
			self.position += 3
			self.risk += 2
		if self.risk >= 5:
			self.stunDuration = 2
			self.risk = 0
		self.remainingTurns -= 1

	def evaluate(self):
		if self.curRisk == "GAME_OVER":
			return 0
		space_gain = self.position - self.starting_position
		risk_gain = self.risk - self.starting_risk
		if self.risk == 4:
			risk_gain *= 2
		elif self.risk == 3:
			risk_gain *= 1.5
		
		if risk_gain == -1 and self.risk <= 1:
			risk_gain = 0
		return (space_gain * 1.5 - risk_gain - 3 * self.stunDuration) / 3 * 100

class Diving:
	def __init__(self, curCombo, curPoints, remainingTurns, moveSequence):
		self.curCombo = curCombo
		self.curPoints = curPoints
		self.remainingTurns = remainingTurns
		self.moveSequence = moveSequence
		self.optimalPoints, self.optimalCombo = self.find_optimal(TURN_SIMULATED)

	def playTurn(self, action):
		if self.remainingTurns == 0 or self.moveSequence == "GAME_OVER":
			return
		d = {"LEFT": "L", "RIGHT": "R", "DOWN": "D", "UP": "U"}
		if d[action] == self.moveSequence[-self.remainingTurns]:
			self.curCombo += 1
		else:
			self.curCombo = 1
		self.curPoints += self.curCombo
	
	def find_optimal(self, nb_turn):
		points = self.curPoints
		combo = self.curCombo
		for i in range(min(nb_turn, self.remainingTurns)):
			combo += 1
			points += combo
		return (points, combo)
	
	def evaluate(self):
		if self.moveSequence == "GAME_OVER":
			return 0
		optimal = (self.optimalPoints + self.optimalCombo * 2) * 0.75
		return (self.curPoints + self.curCombo * 2) / optimal * 100


class Player:
	def __init__(self, hurdle, archery, skating, diving, score):
		self.hurdle = hurdle
		self.archery = archery
		self.skating = skating
		self.diving = diving
		self.score = score

	def playTurn(self, action, turn_nb):
		self.hurdle.playTurn(action, turn_nb)
		self.archery.playTurn(action)
		self.skating.playTurn(action, turn_nb)
		self.diving.playTurn(action)

	def playTurns(self, actions):
		for i, action in enumerate(actions):
			self.playTurn(action, i + 1)

	def evaluate(self):
		return self.hurdle.evalutate() * game_multiplier(self.score[0]) + self.archery.evaluate() * game_multiplier(self.score[1]) + self.skating.evaluate() * game_multiplier(self.score[2]) + self.diving.evaluate() * game_multiplier(self.score[3])

def createGameArray(idx, gpu, reg_0, reg_1, reg_2, reg_3, reg_4, reg_5, reg_6):
	# hurdle
	game = []
	if idx == 0:
		game.append(Hurdle(reg_0, reg_3, gpu))
		game.append(Hurdle(reg_1, reg_4, gpu))
		game.append(Hurdle(reg_2, reg_5, gpu))
	# archery
	elif idx == 1:
		game.append(Archery([reg_0, reg_1], len(gpu), gpu))
		game.append(Archery([reg_2, reg_3], len(gpu), gpu))
		game.append(Archery([reg_4, reg_5], len(gpu), gpu))
	elif idx == 2:
		game.append(Skating(reg_0, reg_3 if reg_3 > 0 else 0, -reg_3 if reg_3 < 0 else 0, reg_6, gpu))
		game.append(Skating(reg_1, reg_4 if reg_4 > 0 else 0, -reg_4 if reg_4 < 0 else 0, reg_6, gpu))
		game.append(Skating(reg_2, reg_5 if reg_5 > 0 else 0, -reg_5 if reg_5 < 0 else 0, reg_6, gpu))
	else:
		game.append(Diving(reg_3, reg_0, len(gpu), gpu))
		game.append(Diving(reg_4, reg_1, len(gpu), gpu))
		game.append(Diving(reg_5, reg_2, len(gpu), gpu))
	return game

# game with a lower score need to have a higher multiplier as gaining points here will have more impact
def game_multiplier(game_score):
	if game_score == 0:
		return 100
	else:
		return 50 // game_score

player_idx = int(input())
nb_games = int(input())

actions = ["UP", "DOWN", "LEFT", "RIGHT"]
# game loop
while True:
	timestamp = time.time()
	scores = []
	for i in range(3):
		score_info = input()
		break_score = list(map(int, score_info.split()))
		score_player = []
		for i in range(1, 11, 3):
			score_player.append(break_score[i] * 3 + break_score[i + 1])
		scores.append(score_player)
	games = []
	for i in range(nb_games):
		cur_game_per_player = []
		inputs = input().split()
		gpu = inputs[0]
		reg_0 = int(inputs[1])
		reg_1 = int(inputs[2])
		reg_2 = int(inputs[3])
		reg_3 = int(inputs[4])
		reg_4 = int(inputs[5])
		reg_5 = int(inputs[6])
		reg_6 = int(inputs[7])
		games.append(createGameArray(i, gpu, reg_0, reg_1, reg_2, reg_3, reg_4, reg_5, reg_6))
		if i == 1:
			print(reg_1, reg_2, file=sys.stderr)
		
	players = []
	for i in range(3):
		players.append(Player(games[0][i], games[1][i], games[2][i], games[3][i], scores[i]))
	
	# for now simulation only consider our player
	myself = players[player_idx]
	best_score = (float("-inf"), None)
	for comb in itertools.product(actions, repeat=TURN_SIMULATED):
		copy_of_myself = copy.deepcopy(myself)
		copy_of_myself.playTurns(comb)
		score = copy_of_myself.evaluate()
		if score > best_score[0]:
			best_score = (score, comb)
	print(time.time() - timestamp, file=sys.stderr)
	print(best_score[1][0])