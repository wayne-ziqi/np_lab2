//
// Created by ziqi on 2023/3/25.
//

#ifndef WORDGAME_COMPETITION_H
#define WORDGAME_COMPETITION_H

#include "Player.h"
#include <string>
#include <list>

namespace GServer {
#define MAX_STAMINA 3

    enum GMove {
        Empty = 0, Rock, Paper, Scissor
    };

    class Competition {
    private:
        int id;
        Player *player1;
        Player *player2;
        std::list<std::pair<GMove, GMove>> compHistory;
        int score1, score2;
        GMove lastMov;

        // 4 state: state0: player1 & player2 online; state1: player1 attacked, wait for player2's reaction
        // state2: player2 attacked, wait for player1's reaction, state4: finished
        int compState;

        // state 0:
        int launchState;

        [[nodiscard]] int getPlayerNo(const std::string &name) const;

        static int testMove(GMove move1, GMove move2);

    public:
        Competition(int compID) : player1(nullptr), player2(nullptr),
                                  id(compID),
                                  score1(MAX_STAMINA),
                                  score2(MAX_STAMINA),
                                  lastMov(Empty),
                                  compState(0), launchState(0) {}

        void setPlayer(Player &player, int no);

        void advanceCompState(const std::string &from_player, GMove gMove);

        GMove getLatestMove(const std::string &player);

        Player* getTheOther(const std::string &player);
        /**
         * transfer launch state according to the player's attitude
         * @param player
         * @param attitude 2: accept, 3: reject
         */
        void advanceLchState(Player &player, int attitude);

        bool isFinished() const;

        bool isCool()const;

        bool isReady() const;

        bool isDeclined() const;

        int getID() const { return id; }

        int getResult(const std::string &player);

        int getLchState() const { return launchState; }

        int getCompState() const { return compState; }

        int getStamina(const std::string &player);

    };
}

#endif //WORDGAME_COMPETITION_H
