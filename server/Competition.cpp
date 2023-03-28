//
// Created by ziqi on 2023/3/25.
//

#include "Competition.h"

#include "ServerException.h"


int GServer::Competition::getPlayerNo(const std::string &name) const {
    if (name == player1->get_name()) return 1;
    if (name == player2->get_name()) return 2;
    assert(0);
}

void GServer::Competition::advanceCompState(const std::string &from_player, GServer::GMove gMove) {
    assert(gMove != Empty);
    int pno = getPlayerNo(from_player);
    switch (compState) {
        case 0: {
            assert(lastMov == Empty);
            lastMov = gMove, compState = pno;
            break;
        }
        case 1: {
            assert(lastMov != Empty);
            if (pno == 1)
                throw SException(PLAY1_DUP_MV, "player1 waiting for player2, can't move again");
            int test = testMove(lastMov, gMove);
            if (test == 1) --score2;
            else if (test == 2) --score1;
            else --score1, --score2;
            compHistory.emplace_back(std::make_pair(lastMov, gMove));
            lastMov = Empty;
            if (isFinished())compState = 3;
            else compState = 0;
            break;
        }
        case 2: {
            assert(lastMov != Empty);
            if (pno == 2)
                throw SException(PLAY2_DUP_MV, "player2 waiting for player1, can't move again");
            int test = testMove(gMove, lastMov);
            if (test == 1) --score2;
            else if (test == 2) --score1;
            else --score1, --score2;
            compHistory.emplace_back(std::make_pair(gMove, lastMov));
            lastMov = Empty;
            if (isFinished())compState = 3;
            else compState = 0;
            break;
        }
        case 3: {
            // loop
            break;
        }
        default:
            assert(0);
    }
}

void GServer::Competition::advanceLchState(Player &player, int attitude) {
    assert(attitude == 2 || attitude == 3);
    switch (launchState) {
        case 0: {
            // a total fresh competition, need add the player to player1
            assert(attitude == 2);
            setPlayer(player, 1);
            launchState = 1;
            break;
        }
        case 1: {
            // player1 is in, waiting for player 2
            assert(player1 != &player);
            setPlayer(player, 2);
            launchState = attitude;
            break;
        }
        case 2:
        case 3: {
            // 2: player2 has accepted, competition is online
            // 3: player2 has declined, competition should be canceled
            break;
        }
        default:
            assert(0);
    }
}


int GServer::Competition::testMove(GServer::GMove move1, GServer::GMove move2) {
    if (move1 == move2)return 0;
    if (move1 == Scissor && move2 == Paper) return 1;
    if (move1 == Paper && move2 == Rock) return 1;
    if (move1 == Rock && move2 == Scissor) return 1;
    return 2;
}

bool GServer::Competition::isFinished() const {
    return compState == 3 || score1 <= 0 || score2 <= 0;
}

bool GServer::Competition::isCool() const {
    return compState == 0x00;
}

void GServer::Competition::setPlayer(GServer::Player &player, int no) {
    if (no == 1) player1 = &player;
    else if (no == 2) player2 = &player;
    else
        assert(0);
}

bool GServer::Competition::isReady() const {
    return launchState == 2;
}

bool GServer::Competition::isDeclined() const {
    return launchState == 3;
}

int GServer::Competition::getResult(const std::string &player) {
    if (player == player1->get_name()) {
        if (score1 > score2) return 0x01;
        if (score1 < score2) return 0x02;
        return 0x03;
    } else if (player == player2->get_name()) {
        if (score1 < score2) return 0x01;
        if (score1 > score2) return 0x02;
        return 0x03;
    }
    assert(0);
}

GServer::GMove GServer::Competition::getLatestMove(const std::string &player) {
    if (compHistory.empty())return Empty;
    int no = getPlayerNo(player);
    if (no == 1) return compHistory.back().first;
    else return compHistory.back().second;
}

int GServer::Competition::getStamina(const std::string &player) {
    if (getPlayerNo(player) == 1) return score1;
    else return score2;
}

GServer::Player *GServer::Competition::getTheOther(const std::string &player) {
    if (player == player1->get_name())return player2;
    else if (player == player2->get_name()) return player1;
    assert(0);
}



