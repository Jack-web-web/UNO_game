#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <queue>

using namespace cv;
using namespace std;

// UNO牌类
class UnoCard {
public:
    enum Color { RED, YELLOW, GREEN, BLUE, WILD };
    enum Type { NUMBER, SKIP, REVERSE, DRAW_TWO, WILD_COLOR, WILD_DRAW_FOUR };

    UnoCard(Color c, Type t, int num = 0) : color(c), type(t), number(num) {
        // 初始化牌面图像
        initCardImage();
    }

    // 获取牌的字符串表示
    string toString() const {
        string colors[] = { "红", "黄", "绿", "蓝", "野生" };
        string types[] = { "数字", "跳过", "反转", " Draw Two", "野生颜色", " Draw Four" };

        if (type == NUMBER) {
            return colors[color] + " " + to_string(number);
        }
        else if (type == WILD_COLOR || type == WILD_DRAW_FOUR) {
            return types[type];
        }
        else {
            return colors[color] + " " + types[type];
        }
    }

    // 获取牌面图像
    Mat getImage() const {
        return cardImage;
    }

    // 获取牌的颜色
    Color getColor() const {
        return color;
    }

    // 获取牌的类型
    Type getType() const {
        return type;
    }

    // 获取牌的数字（仅当牌是数字牌时有效）
    int getNumber() const {
        return number;
    }

    // 设置牌的颜色（仅用于野生牌）
    void setColor(Color c) {
        if (type == WILD_COLOR || type == WILD_DRAW_FOUR) {
            color = c;
            // 更新牌面图像
            initCardImage();
        }
    }

    // 检查这张牌是否可以放在另一张牌上面
    bool canBePlacedOn(const UnoCard& other) const {
        if (type == WILD_COLOR || type == WILD_DRAW_FOUR) {
            return true; // 野生牌可以放在任何牌上
        }

        if (color == other.color) {
            return true;
        }

        if (type == other.type && type != NUMBER) {
            return true;
        }

        if (type == NUMBER && other.type == NUMBER && number == other.number) {
            return true;
        }

        return false;
    }

private:
    Color color;
    Type type;
    int number; // 仅用于数字牌
    Mat cardImage;

    // 初始化牌面图像
    void initCardImage() {
        // 创建牌的基本形状
        cardImage = Mat(120, 80, CV_8UC3, Scalar(255, 255, 255));
        rectangle(cardImage, Point(1, 1), Point(78, 118), Scalar(0, 0, 0), 2);

        // 设置牌的背景颜色
        Scalar bgColor;
        switch (color) {
        case RED: bgColor = Scalar(0, 0, 255); break;
        case YELLOW: bgColor = Scalar(0, 255, 255); break;
        case GREEN: bgColor = Scalar(0, 255, 0); break;
        case BLUE: bgColor = Scalar(255, 0, 0); break;
        case WILD: bgColor = Scalar(180, 105, 255); break;
        }

        // 填充牌的背景
        rectangle(cardImage, Point(3, 3), Point(76, 116), bgColor, -1);

        // 绘制牌面信息
        string text;
        if (type == NUMBER) {
            text = to_string(number);
        }
        else if (type == SKIP) {
            text = "X";
        }
        else if (type == REVERSE) {
            text = "B";
        }
        else if (type == DRAW_TWO) {
            text = "+2";
        }
        else if (type == WILD_COLOR) {
            text = "WC";
        }
        else if (type == WILD_DRAW_FOUR) {
            text = "+4";
        }

        // 设置文本颜色
        Scalar textColor = (color == YELLOW || color == GREEN) ? Scalar(0, 0, 0) : Scalar(255, 255, 255);

        // 在牌中间绘制文本
        int fontFace = FONT_HERSHEY_SIMPLEX;
        double fontScale = 1.5;
        int thickness = 2;
        int baseline = 0;
        Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
        Point textOrg((cardImage.cols - textSize.width) / 2, (cardImage.rows + textSize.height) / 2);
        putText(cardImage, text, textOrg, fontFace, fontScale, textColor, thickness);
    }
};

// UNO牌堆类
class UnoDeck {
private:
    vector<UnoCard> cards;

public:
    UnoDeck() {
        // 初始化一副标准UNO牌
        initializeDeck();
        shuffle();
    }

    void addCard(const UnoCard& card) {
        cards.push_back(card);
    }

    // 初始化牌堆
    void initializeDeck() {
        // 清空牌堆
        cards.clear();

        // 添加数字牌 (0-9)
        for (int color = 0; color < 4; color++) { // 四种颜色
            // 每个颜色有一个0
            cards.push_back(UnoCard(static_cast<UnoCard::Color>(color), UnoCard::NUMBER, 0));

            // 每个颜色的1-9各有两个
            for (int num = 1; num <= 9; num++) {
                cards.push_back(UnoCard(static_cast<UnoCard::Color>(color), UnoCard::NUMBER, num));
                cards.push_back(UnoCard(static_cast<UnoCard::Color>(color), UnoCard::NUMBER, num));
            }
        }

        // 添加功能牌
        for (int color = 0; color < 4; color++) { // 四种颜色
            // 每个颜色有两个Skip
            for (int i = 0; i < 2; i++) {
                cards.push_back(UnoCard(static_cast<UnoCard::Color>(color), UnoCard::SKIP));
            }

            // 每个颜色有两个Reverse
            for (int i = 0; i < 2; i++) {
                cards.push_back(UnoCard(static_cast<UnoCard::Color>(color), UnoCard::REVERSE));
            }

            // 每个颜色有两个Draw Two
            for (int i = 0; i < 2; i++) {
                cards.push_back(UnoCard(static_cast<UnoCard::Color>(color), UnoCard::DRAW_TWO));
            }
        }

        // 添加野生牌
        // 四张Wild Color
        for (int i = 0; i < 4; i++) {
            cards.push_back(UnoCard(UnoCard::WILD, UnoCard::WILD_COLOR));
        }

        // 四张Wild Draw Four
        for (int i = 0; i < 4; i++) {
            cards.push_back(UnoCard(UnoCard::WILD, UnoCard::WILD_DRAW_FOUR));
        }
    }

    // 洗牌
    void shuffle() {
        srand(static_cast<unsigned int>(time(nullptr)));
        random_shuffle(cards.begin(), cards.end());
    }

    // 从牌堆顶部抽一张牌
    UnoCard drawCard() {
        if (cards.empty()) {
            throw runtime_error("牌堆已空!");
        }

        UnoCard card = cards.back();
        cards.pop_back();
        return card;
    }

    // 检查牌堆是否为空
    bool isEmpty() const {
        return cards.empty();
    }

    // 获取牌堆中剩余牌的数量
    int size() const {
        return cards.size();
    }
};

// UNO玩家类
class UnoPlayer {
private:
    string name;
    vector<UnoCard> hand;

public:
    UnoPlayer(const string& n) : name(n) {}

    // 添加一张牌到玩家手中
    void addCard(const UnoCard& card) {
        hand.push_back(card);
    }

    // 从玩家手中移除一张牌
    void removeCard(int index) {
        if (index >= 0 && index < hand.size()) {
            hand.erase(hand.begin() + index);
        }
    }

    // 显示玩家手中的牌
    void showHand() const {
        cout << name << "的手牌: ";
        for (size_t i = 0; i < hand.size(); i++) {
            cout << "[" << i + 1 << "] " << hand[i].toString() << " ";
        }
        cout << endl;
    }

    // 获取玩家手中的牌
    const vector<UnoCard>& getHand() const {
        return hand;
    }

    // 获取玩家名称
    string getName() const {
        return name;
    }

    // 检查玩家是否有UNO（只剩一张牌）
    bool hasUno() const {
        return hand.size() == 1;
    }

    // 检查玩家是否获胜（没有牌了）
    bool hasWon() const {
        return hand.empty();
    }

    // 获取玩家手中可打出的牌的索引
    vector<int> getPlayableCards(const UnoCard& topCard) const {
        vector<int> playableIndices;
        for (size_t i = 0; i < hand.size(); i++) {
            if (hand[i].canBePlacedOn(topCard)) {
                playableIndices.push_back(i);
            }
        }
        return playableIndices;
    }
};

// UNO游戏类
class UnoGame {
private:
    UnoDeck deck;
    vector<UnoPlayer> players;
    queue<UnoCard> discardPile;
    int currentPlayerIndex;
    bool gameOver;
    bool clockwise; // 游戏方向：顺时针或逆时针

public:
    UnoGame() : gameOver(false), clockwise(true) {
        // 初始化游戏
        initializeGame();
    }

    // 初始化游戏
    void initializeGame() {
        // 创建玩家
        players.clear();
        players.push_back(UnoPlayer("玩家"));
        players.push_back(UnoPlayer("电脑1"));
        players.push_back(UnoPlayer("电脑2"));
        players.push_back(UnoPlayer("电脑3"));

        // 洗牌
        deck.shuffle();

        // 给每个玩家发7张牌
        for (int i = 0; i < 7; i++) {
            for (auto& player : players) {
                player.addCard(deck.drawCard());
            }
        }

        // 翻开第一张牌作为起始牌
        UnoCard startCard = deck.drawCard();
        // 如果起始牌是功能牌，重新抽牌直到是数字牌
        while (startCard.getType() != UnoCard::NUMBER) {
            deck.addCard(startCard);
            deck.shuffle();
            startCard = deck.drawCard();
        }
        discardPile.push(startCard);

        // 设置当前玩家为第一个玩家
        currentPlayerIndex = 0;
    }

    // 显示游戏状态
    void displayGameState() {
        system("cls"); // 清屏（Windows系统使用system("cls")）

        // 创建游戏窗口
        Mat gameWindow = Mat(600, 800, CV_8UC3, Scalar(0, 100, 0));

        // 显示当前玩家
        string currentPlayerText = "now player: " ;
        if (players[currentPlayerIndex].getName() == "玩家") {
            currentPlayerText += "player";
        }
        else if (players[currentPlayerIndex].getName() == "电脑1") {
            currentPlayerText += "computer1";
        }
        else if (players[currentPlayerIndex].getName() == "电脑2") {
            currentPlayerText += "computer2";
        }
        else if (players[currentPlayerIndex].getName() == "电脑3") {
            currentPlayerText += "computer3";
        }
        putText(gameWindow, currentPlayerText, Point(300, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);

        // 显示弃牌堆顶部的牌
        putText(gameWindow, "throw away", Point(350, 150), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 1);
        Mat topCardImg = discardPile.back().getImage();
        Mat roi = gameWindow(Rect(360, 170, topCardImg.cols, topCardImg.rows));
        topCardImg.copyTo(roi);

        // 显示玩家的手牌
        putText(gameWindow, "in your hand", Point(350, 350), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 1);

        const vector<UnoCard>& playerHand = players[0].getHand();
        for (size_t i = 0; i < playerHand.size(); i++) {
            Mat cardImg = playerHand[i].getImage();
            Mat roi = gameWindow(Rect(50 + i * 90, 380, cardImg.cols, cardImg.rows));
            cardImg.copyTo(roi);

            // 显示牌的索引
            string indexText = to_string(i + 1);
            putText(gameWindow, indexText, Point(50 + i * 90, 370), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
        }

        // 显示操作提示
        putText(gameWindow, "Press the number button to select the card you want to play, and press the D button to draw the card", Point(0, 550), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 255, 255), 1);

        // 显示窗口
        imshow("UNO game", gameWindow);
        waitKey(100);
    }

    // 玩家回合
    void playerTurn() {
        UnoPlayer& currentPlayer = players[currentPlayerIndex];
        UnoCard topCard = discardPile.back();

        // 显示游戏状态
        displayGameState();

        // 检查玩家是否有可打出的牌
        vector<int> playableIndices = currentPlayer.getPlayableCards(topCard);

        if (playableIndices.empty()) {
            // 没有可打的牌，必须抽牌
            cout << "你没有可打的牌，必须抽一张牌。" << endl;
            waitKey(1000);

            UnoCard drawnCard = deck.drawCard();
            currentPlayer.addCard(drawnCard);

            cout << "你抽到了: " << drawnCard.toString() << endl;

            // 检查抽到的牌是否可以打
            if (drawnCard.canBePlacedOn(topCard)) {
                cout << "这张牌可以打! 按Y出牌，按N保留。" << endl;
                int key = waitKey(0);

                if (key == 'y' || key == 'Y') {
                    // 找出抽到的牌在玩家手中的索引
                    int cardIndex = currentPlayer.getHand().size() - 1;
                    playCard(currentPlayer, cardIndex);
                }
            }
            else {
                cout << "这张牌不能打，轮到下一位玩家。" << endl;
                waitKey(1000);
            }
        }
        else {
            // 有可打的牌，等待玩家选择
            bool cardPlayed = false;

            while (!cardPlayed) {
                int key = waitKey(0);

                // 按数字键选择要出的牌
                if (key >= '1' && key <= '9') {
                    int selectedIndex = key - '1';

                    // 检查选择的牌是否有效
                    if (selectedIndex < currentPlayer.getHand().size()) {
                        if (currentPlayer.getHand()[selectedIndex].canBePlacedOn(topCard)) {
                            playCard(currentPlayer, selectedIndex);
                            cardPlayed = true;
                        }
                        else {
                            cout << "这张牌不能打!" << endl;
                        }
                    }
                }
                // 按D键抽牌
                else if (key == 'd' || key == 'D') {
                    UnoCard drawnCard = deck.drawCard();
                    currentPlayer.addCard(drawnCard);

                    cout << "你抽到了: " << drawnCard.toString() << endl;

                    // 检查抽到的牌是否可以打
                    if (drawnCard.canBePlacedOn(topCard)) {
                        cout << "这张牌可以打! 按Y出牌，按N保留。" << endl;
                        int confirmKey = waitKey(0);

                        if (confirmKey == 'y' || confirmKey == 'Y') {
                            // 找出抽到的牌在玩家手中的索引
                            int cardIndex = currentPlayer.getHand().size() - 1;
                            playCard(currentPlayer, cardIndex);
                            cardPlayed = true;
                        }
                        else {
                            cout << "你选择保留这张牌，轮到下一位玩家。" << endl;
                            waitKey(1000);
                            cardPlayed = true;
                        }
                    }
                    else {
                        cout << "这张牌不能打，轮到下一位玩家。" << endl;
                        waitKey(1000);
                        cardPlayed = true;
                    }
                }
            }
        }
    }

    // 电脑回合
    void computerTurn() {
        UnoPlayer& currentPlayer = players[currentPlayerIndex];
        UnoCard topCard = discardPile.back();
        // 显示游戏状态
        displayGameState();
        cout << currentPlayer.getName() << "的回合..." << endl;
        waitKey(1000);

        // 检查电脑是否有可打出的牌
        vector<int> playableIndices = currentPlayer.getPlayableCards(topCard);

        if (playableIndices.empty()) {
            // 没有可打的牌，必须抽牌
            cout << currentPlayer.getName() << "没有可打的牌，抽一张牌。" << endl;
            waitKey(1000);

            UnoCard drawnCard = deck.drawCard();
            currentPlayer.addCard(drawnCard);

            // 检查抽到的牌是否可以打
            if (drawnCard.canBePlacedOn(topCard)) {
                cout << currentPlayer.getName() << "抽到一张可打的牌，打出它。" << endl;
                waitKey(1000);

                // 找出抽到的牌在玩家手中的索引
                int cardIndex = currentPlayer.getHand().size() - 1;
                playCard(currentPlayer, cardIndex);
            }
            else {
                cout << currentPlayer.getName() << "抽到的牌不能打，跳过回合。" << endl;
                waitKey(1000);
            }
        }
        else {
            // 有可打的牌，选择一张打出
            // 简单AI：优先选择功能牌，然后是高数字牌
            int bestCardIndex = -1;
            int highestValue = -1;

            for (int index : playableIndices) {
                const UnoCard& card = currentPlayer.getHand()[index];
                int value = 0;

                // 根据牌的类型和颜色分配权重
                if (card.getType() == UnoCard::WILD_DRAW_FOUR) {
                    value = 5;
                }
                else if (card.getType() == UnoCard::DRAW_TWO) {
                    value = 4;
                }
                else if (card.getType() == UnoCard::SKIP) {
                    value = 3;
                }
                else if (card.getType() == UnoCard::REVERSE) {
                    value = 3;
                }
                else if (card.getType() == UnoCard::WILD_COLOR) {
                    value = 2;
                }
                else {
                    // 数字牌，值越高越好
                    value = card.getNumber();
                }

                // 尝试匹配当前颜色
                if (card.getColor() == topCard.getColor()) {
                    value += 1;
                }

                if (value > highestValue) {
                    highestValue = value;
                    bestCardIndex = index;
                }
            }

            if (bestCardIndex != -1) {
                cout << currentPlayer.getName() << "打出: " << currentPlayer.getHand()[bestCardIndex].toString() << endl;
                waitKey(1000);
                playCard(currentPlayer, bestCardIndex);
            }
        }
    }

    // 打出一张牌
    void playCard(UnoPlayer& player, int cardIndex) {
        UnoCard card = player.getHand()[cardIndex];
        player.removeCard(cardIndex);

        // 将牌放入弃牌堆
        discardPile.push(card);

        // 检查玩家是否获胜
        if (player.hasWon()) {
            cout << player.getName() << "获胜!" << endl;
            gameOver = true;
            return;
        }

        // 检查玩家是否只剩一张牌（UNO）
        if (player.hasUno()) {
            cout << player.getName() << "喊UNO!" << endl;
            waitKey(1000);
        }

        // 处理功能牌的效果
        switch (card.getType()) {
        case UnoCard::SKIP:
            cout << "跳过下一位玩家的回合!" << endl;
            waitKey(1000);
            nextPlayer();
            break;

        case UnoCard::REVERSE:
            cout << "游戏方向反转!" << endl;
            waitKey(1000);
            clockwise = !clockwise;
            break;

            {
        case UnoCard::DRAW_TWO:
            cout << "下一位玩家必须抽两张牌并跳过回合!" << endl;
            waitKey(1000);

            // 下一位玩家抽两张牌
            int nextPlayerIndex = getNextPlayerIndex();
            UnoPlayer& nextPlayer = players[nextPlayerIndex];

            for (int i = 0; i < 2; i++) {
                if (!deck.isEmpty()) {
                    nextPlayer.addCard(deck.drawCard());
                }
            }

            cout << nextPlayer.getName() << "抽了两张牌。" << endl;
            waitKey(1000);

            // 跳过下一位玩家的回合
            this->nextPlayer();
            break;
            }

            {
        case UnoCard::WILD_COLOR:
            cout << "选择一种颜色: 1-红, 2-黄, 3-绿, 4-蓝" << endl;
            // 玩家选择颜色
            if (player.getName() == "玩家") {
                int key;
                do {
                    key = waitKey(0);
                } while (key < '1' || key > '4');

                UnoCard::Color newColor = static_cast<UnoCard::Color>(key - '1');
                card.setColor(newColor);

                // 更新弃牌堆中的牌
                discardPile.pop();
                discardPile.push(card);

                cout << "你选择了: " << getColorName(newColor) << endl;
            }
            else {
                // 电脑随机选择一种颜色
                UnoCard::Color newColor = static_cast<UnoCard::Color>(rand() % 4);
                card.setColor(newColor);

                // 更新弃牌堆中的牌
                discardPile.pop();
                discardPile.push(card);

                cout << player.getName() << "选择了: " << getColorName(newColor) << endl;
            }
            waitKey(1000);
            break;
            }

            {
        case UnoCard::WILD_DRAW_FOUR:
            cout << "下一位玩家必须抽四张牌并跳过回合!" << endl;
            waitKey(1000);

            // 玩家选择颜色
            if (player.getName() == "玩家") {
                cout << "选择一种颜色: 1-红, 2-黄, 3-绿, 4-蓝" << endl;
                int key;
                do {
                    key = waitKey(0);
                } while (key < '1' || key > '4');

                UnoCard::Color newColor = static_cast<UnoCard::Color>(key - '1');
                card.setColor(newColor);

                // 更新弃牌堆中的牌
                discardPile.pop();
                discardPile.push(card);

                cout << "你选择了: " << getColorName(newColor) << endl;
            }
            else {
                // 电脑随机选择一种颜色
                UnoCard::Color newColor = static_cast<UnoCard::Color>(rand() % 4);
                card.setColor(newColor);

                // 更新弃牌堆中的牌
                discardPile.pop();
                discardPile.push(card);

                cout << player.getName() << "选择了: " << getColorName(newColor) << endl;
            }
            waitKey(1000);

            // 下一位玩家抽四张牌
            int drawFourPlayerIndex = getNextPlayerIndex();
            UnoPlayer& drawFourPlayer = players[drawFourPlayerIndex];

            for (int i = 0; i < 4; i++) {
                if (!deck.isEmpty()) {
                    drawFourPlayer.addCard(deck.drawCard());
                }
            }

            cout << drawFourPlayer.getName() << "抽了四张牌。" << endl;
            waitKey(1000);

            // 跳过下一位玩家的回合
            this->nextPlayer();
            break;
            }

        default:
            // 数字牌，没有特殊效果
            break;
        }
    }

    // 获取下一位玩家的索引
    int getNextPlayerIndex() const {
        int nextIndex = currentPlayerIndex;

        if (clockwise) {
            nextIndex = (nextIndex + 1) % players.size();
        }
        else {
            nextIndex = (nextIndex - 1 + players.size()) % players.size();
        }

        return nextIndex;
    }

    // 转到下一位玩家
    void nextPlayer() {
        currentPlayerIndex = getNextPlayerIndex();
    }

    // 运行游戏
    void run() {
        // 显示欢迎信息
        Mat welcomeWindow = Mat(300, 500, CV_8UC3, Scalar(0, 100, 0));
        putText(welcomeWindow, "Welcome to UNO game", Point(100, 100), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 2);
        putText(welcomeWindow, "Press any key to start the game...", Point(120, 200), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 1);
        imshow("UNO游戏", welcomeWindow);
        waitKey(0);
        destroyWindow("UNO游戏");

        // 游戏主循环
        while (!gameOver) {
            // 检查牌堆是否为空
            if (deck.isEmpty()) {
                // 重新洗牌
                cout << "牌堆已空，重新洗牌..." << endl;
                waitKey(1000);

                // 保留弃牌堆顶部的牌，其余的牌重新洗入牌堆
                UnoCard topCard = discardPile.back();
                discardPile.pop();

                while (!discardPile.empty()) {
                    deck.addCard(discardPile.front());
                    discardPile.pop();
                }

                discardPile.push(topCard);
                deck.shuffle();
            }

            // 当前玩家回合
            if (players[currentPlayerIndex].getName() == "玩家") {
                playerTurn();
            }
            else {
                computerTurn();
            }

            // 如果游戏没有结束，转到下一位玩家
            if (!gameOver) {
                nextPlayer();
            }
        }

        // 显示游戏结果
        showResult();
    }

    // 显示游戏结果
    void showResult() {
        Mat resultWindow = Mat(300, 500, CV_8UC3, Scalar(0, 100, 0));

        string resultText = "";
        if (players[currentPlayerIndex].getName() == "玩家") {
            resultText += "player";
        }
        else if (players[currentPlayerIndex].getName() == "电脑1") {
            resultText += "computer1";
        }
        else if (players[currentPlayerIndex].getName() == "电脑2") {
            resultText += "computer2";
        }
        else if (players[currentPlayerIndex].getName() == "电脑3") {
            resultText += "computer3";
        }
        resultText += " win!";
        putText(resultWindow, "Game over", Point(150, 50), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 2);
        putText(resultWindow, resultText, Point(150, 150), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 2);
        putText(resultWindow, "Press any key to exit...", Point(150, 250), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 1);

        imshow("游戏结果", resultWindow);
        waitKey(0);
        destroyWindow("游戏结果");
    }

    // 获取颜色名称
    string getColorName(UnoCard::Color color) const {
        string colorNames[] = { "红", "黄", "绿", "蓝" };
        return colorNames[color];
    }

    // 检查游戏是否结束
    bool isGameOver() const {
        return gameOver;
    }
};

int main() {
    // 创建游戏对象
    UnoGame game;

    // 运行游戏
    game.run();

    return 0;
}