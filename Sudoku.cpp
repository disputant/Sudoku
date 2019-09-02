#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <chrono>

class muTimer
{
    using Clock = std::chrono::high_resolution_clock;
    bool active = false;
    Clock::duration   duration_;
    Clock::time_point start_ = Clock::now(), stop_ = Clock::now();

    muTimer(const muTimer&)             = delete;
    muTimer& operator=(const muTimer&)  = delete;
public:
    using ns       = std::chrono::nanoseconds;
    using mks      = std::chrono::microseconds;
    using ms       = std::chrono::milliseconds;
    muTimer() { reset(); start(); }
    ~muTimer() = default;
    muTimer& reset()
    {
        duration_ = std::chrono::nanoseconds(0);
        active    = false;
        return *this;
    }
    muTimer& start()
    {
        if (!active)
        {
            start_ = Clock::now();
            active = true;
        }
        return *this;
    }
    muTimer& stop()
    {
        if (active)
        {
            stop_      = Clock::now();
            duration_ += stop_ - start_;
            active     = false;
        }
        return *this;
    }
    template<typename T = mks>
        unsigned long long duration()
    {
        return static_cast<unsigned long long>
            (std::chrono::duration_cast<T>(stop_-start_).count());
    }
};


class Sudoku
{
public:
    Sudoku(bool clean = true);
    Sudoku(const Sudoku&s)              { memcpy(p,s.p,sizeof(p)); }
    Sudoku(const char *);
    Sudoku& operator = (const Sudoku&s) { memcpy(p,s.p,sizeof(p)); return *this; }
    ~Sudoku() {};

    int    operator()(int r, int c) const;
    int&   operator()(int r, int c) { return p[r-1][c-1]; }
    int    ok        () const;
    void   shuffle   (int count = 3);
    void   out       () const;
    int    zeros     () const;
    Sudoku solidSolve() const;
    Sudoku addZeros  (int max) const;
    Sudoku solve     (int&count) const;

    void Generate();
private:
    bool Gen();
    static void trySolve(Sudoku&s, Sudoku& result, int& count );
    void swapSqR(int i, int j);
    void swapSqC(int i, int j);
    void swapRow(int i, int j);
    void swapCol(int i, int j);
    void permut (int*perm);
    void swap(int&i,int&j) { int t = i; i = j; j = t; }
    int  check(int r, int c) const;       // Если один вариант - какой?
    int  checkCount(int r, int c) const;  // Сколько вариантов
    int p[9][9];
};

inline int  Sudoku::zeros() const /*fold00*/
{
    int z = 0;
    for(int i = 0; i < sizeof(p)/sizeof(int); ++i)
        if (*((int*)p+i) == 0) ++z;
    return z;
}

inline int Sudoku::ok() const /*fold00*/
{
    if (zeros()) return -1;
    static const char cstr[] = "-+++++++++";
    // Проверка горизоналей и вертикалей
    for(int r = 0; r < 9; ++r)
    {
        char h[11] = {0}, v[11] = {0};
        memset(h,'-',10); memset(v,'-',10);
        for(int c = 0; c < 9; ++c)
        {
            h[p[r][c]] = '+';
            v[p[c][r]] = '+';
        }
        if (strcmp(h,cstr)) return 0;
        if (strcmp(v,cstr)) return 0;
    }
    // Проверка квадратов
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
        {
            char s[11] = {0};
            memset(s,'-',10);
            for(int r = i*3; r < i*3+3; ++r)
                for(int c = j*3; c < j*3 + 3; ++c)
                    s[p[r][c]] = '+';
            if (strcmp(s,cstr))
                return 0;
        }
    return 1;
}

inline int Sudoku::check(int r, int c) const /*fold00*/
{
    int allow[9] = {1,2,3,4,5,6,7,8,9};
    // Проверка горизонтали
    for(int x = 0; x < 9; ++x)
    {
        if (p[x][c] && (r != x)) allow[p[x][c]-1] = 0;
        if (p[r][x] && (c != x)) allow[p[r][x]-1] = 0;
    }
    // Проверка квадрата
    int rq = (r/3)*3;
    int cq = (c/3)*3;
    for(int x = 0; x < 3; ++x)
        for(int y = 0; y < 3; ++y)
        {
            int rx = rq + x;
            int cy = cq + y;

            if (p[rx][cy] && ((rx != r) || (cy != c)))
                allow[p[rx][cy]-1] = 0;
        }

    int nonzero = 0;
    int result  = 0;
    for(int i = 0; i < 9; ++i)
        if (allow[i])
        {
            if (++nonzero > 1) break;
            result = allow[i];
        }
    if (nonzero != 1) return 0;
    return result;
}

inline int Sudoku::checkCount(int r, int c) const /*FOLD00*/
{
    int allow[9] = {1,2,3,4,5,6,7,8,9};
    // Проверка горизонтали
    for(int x = 0; x < 9; ++x)
    {
        if (p[x][c] && (r != x)) allow[p[x][c]-1] = 0;
        if (p[r][x] && (c != x)) allow[p[r][x]-1] = 0;
    }
    // Проверка квадрата
    int rq = (r/3)*3;
    int cq = (c/3)*3;
    for(int x = 0; x < 3; ++x)
        for(int y = 0; y < 3; ++y)
        {
            int rx = rq + x;
            int cy = cq + y;

            if (p[rx][cy] && ((rx != r) || (cy != c)))
                allow[p[rx][cy]-1] = 0;
        }

    int nonzero = 0;
    for(int i = 0; i < 9; ++i)
        if (allow[i])
        {
            ++nonzero;
        }
    return nonzero;
}

Sudoku Sudoku::solidSolve() const /*fold00*/
{
    Sudoku s(*this);
    while(s.zeros())
    {
        bool found = false;
        for(int i = 0; i < 9; ++i)
            for(int j = 0; j < 9; ++j)
                if (s.p[i][j] == 0)
                {
                    int res = s.check(i,j);
                    if (res)
                    {
                        found = true;
                        s.p[i][j] = res;
                    }
                }
        if (!found) break;
    }
    return s;
}



Sudoku::Sudoku(const char * s) /*FOLD00*/
{
    for(int r = 0; r < 9; ++r)
        for(int c = 0; c < 9; ++c)
        {
            while(!isdigit(*s)) ++s;
            p[r][c] = *s - '0';
            ++s;
        }
}


Sudoku::Sudoku(bool clean) /*FOLD00*/
{
    static int sd[9][9] = {
        {1,2,3,4,5,6,7,8,9},
        {4,5,6,7,8,9,1,2,3},
        {7,8,9,1,2,3,4,5,6},
        {2,3,1,5,6,4,8,9,7},
        {5,6,4,8,9,7,2,3,1},
        {8,9,7,2,3,1,5,6,4},
        {3,1,2,6,4,5,9,7,8},
        {6,4,5,9,7,8,3,1,2},
        {9,7,8,3,1,2,6,4,5}
    };

    if (clean)
    {
        for(int i = 0; i < 9; ++i)
            for(int j = 0; j < 9; ++j)
                p[i][j] = 0;
    }
    else
    {
        Generate();
        //for(int i = 0; i < 9; ++i)
        //    for(int j = 0; j < 9; ++j)
        //        p[i][j] = sd[i][j];
        //shuffle();
    }
}

void Sudoku::permut (int*perm) /*FOLD00*/
{
    for(int i = 0; i < 9; ++i)
        for(int j = 0; j < 9; ++j)
            p[i][j] = perm[p[i][j]-1];
}

void Sudoku::out() const /*fold00*/
{
    for(int i = 0; i < 9; ++i)
    {
        for(int j = 0; j < 9; ++j)
            printf("%d",p[i][j]);
        printf("\n");
    }
}

void Sudoku::shuffle(int count) /*FOLD00*/
{
    int perm[9] = {1,2,3,4,5,6,7,8,9};
    for(int k = 8; k > 0; --k)
    {
        swap(perm[k],perm[rand()%k]);
    }
    int i,j,l;
    for(int k = 0; k < count; k++)
    {
        i = rand()%3; j = rand()%3;  swapSqR(i,j);
        i = rand()%3; j = rand()%3;  swapSqC(i,j);
        i = rand()%3; j = rand()%3;  l = rand()%3; swapRow(i+l*3,j+l*3);
        i = rand()%3; j = rand()%3;  l = rand()%3; swapCol(i+l*3,j+l*3);
        permut(perm);
    }
}

void Sudoku::swapSqR(int i, int j) /*fold00*/
{
    if ((i>=0)&&(i<=2)&&
        (j>=0)&&(j<=2)&&
        (i != j))
    {
        for(int r = 0; r < 3; ++r)
            for(int c = 0; c < 9; ++c)
            {
                swap(p[i*3+r][c],p[j*3+r][c]);
            }
    }
}
void Sudoku::swapSqC(int i, int j) /*fold00*/
{
    if ((i>=0)&&(i<=2)&&
        (j>=0)&&(j<=2)&&
        (i != j))
    {
        for(int c = 0; c < 3; ++c)
            for(int r = 0; r < 9; ++r)
            {
                swap(p[r][i*3+c],p[r][j*3+c]);
            }
    }
}
void Sudoku::swapRow(int i, int j) /*fold00*/
{
    if ((i>=0)&&(i<=8)&&
        (j>=0)&&(j<=8)&&
        (i/3 == j/3)&&
        (i != j))
    {
        for(int c = 0; c < 9; ++c)
            swap(p[i][c],p[j][c]);
    }
}
void Sudoku::swapCol(int i, int j) /*fold00*/
{
    if ((i>=0)&&(i<=8)&&
        (j>=0)&&(j<=8)&&
        (i/3 == j/3)&&
        (i != j))
    {
        for(int r = 0; r < 9; ++r)
            swap(p[r][i],p[r][j]);
    }
}

int  Sudoku::operator()(int r, int c) const /*fold00*/
{
    if (r < 0) return -1;
    if (c < 0) return -1;
    if (r > 8) return -1;
    if (c > 8) return -1;
    return p[r][c];
}

Sudoku Sudoku::addZeros(int max) const /*FOLD00*/
{
    Sudoku s(*this); // Возвращаемый
    while(s.zeros() < max)
    {
        int nonzeros = 81 - s.zeros();
        bool added = false;
        //printf("Zeros = %d\n",s.zeros());
        for(int i = 0; i < nonzeros; ++i) // Попыток...
        {
            Sudoku t(s); // Временный
            int q = rand()%nonzeros;
            for(int r = 0; r < 9; ++r)
            {
                for(int c = 0; c < 9; ++c)
                {
                    if (t.p[r][c] == 0) continue;
                    if (q > 0)
                    {
                        --q;
                        continue;
                    }
                    t.p[r][c] = 0;
                    int count = 1;
                    //Sudoku sol = t.solidSolve();
                    Sudoku sol = t.solidSolve();
                    if ((sol.ok() == 1)&&(count == 1))
                    {
                        added = true;
                        //printf("s.zeros() = %d\n",s.zeros());
                        //printf("Added...\n");
                        s = t;
                        //printf("s.zeros() = %d\n",s.zeros());
                        break;
                    }
                }
                if (added) break;
            }
            if (added) break;
        }
        if (!added) // Однозначно не идет... пробуем иначе
        {
            for(int i = 0; i < nonzeros; ++i) // Попыток...
            {
                Sudoku t(s); // Временный
                int q = rand()%nonzeros;
                for(int r = 0; r < 9; ++r)
                {
                    for(int c = 0; c < 9; ++c)
                    {
                        if (t.p[r][c] == 0) continue;
                        if (q > 0)
                        {
                            --q;
                            continue;
                        }
                        t.p[r][c] = 0;
                        int count;
                        Sudoku sol = t.solve(count);
                        if ((sol.ok() == 1)&&(count == 1))
                        {
                            added = true;
                            //printf("s.zeros() = %d\n",s.zeros());
                            //printf("Added...\n");
                            s = t;
                            //printf("s.zeros() = %d\n",s.zeros());
                            break;
                        }
                    }
                    if (added) break;
                }
                if (added) break;
            }
        }
        if (!added) break;
    }
    return s;
}


Sudoku Sudoku::solve(int&count) const /*FOLD00*/
{
    Sudoku s = solidSolve();
    if (s.ok() == 1)
    {
        count = 1;
        return s;
    }
    s = *this;
    count = 0;
    Sudoku res = s;
    trySolve(s,res,count);
    return res;
}

int level = 0;

void Sudoku::trySolve(Sudoku&s, Sudoku&res, int&count) /*FOLD00*/
{
    if (s.zeros() == 0) return; // Уже решил
    if (count > 1)      return;

    ++level;
    //printf("             Level %3d, zeros %3d, count %2d\r",level,s.zeros(), count);
    //fflush(stdout);

    // Ищем минимальный
    int r = -1, c = -1;
    int cnt = 9;
    for(int x = 0; x < 9; ++x)
    {
        for(int y = 0; y < 9; ++y)
        {
            if (s.p[x][y]) continue;
            int count = s.checkCount(x,y);
            if (cnt >= count)
            {
                r = x;
                c = y;
                cnt = count;
            }
        }
    }

    assert((r >= 0) && (c >= 0));

    // Составляем массив допустимых значений
    int allow[9] = { 1,2,3,4,5,6,7,8,9 };
    for(int x = 0; x < 9; ++x)
    {
        if (s.p[r][x]) allow[s.p[r][x]-1] = 0;
        if (s.p[x][c]) allow[s.p[x][c]-1] = 0;
    }
    for(int x = 0; x < 3; ++x)
        for(int y = 0; y < 3; ++y)
            if (s.p[(r/3)*3+x][(c/3)*3+y])
                allow[s.p[(r/3)*3+x][(c/3)*3+y]-1] = 0;

    for(int i = 0; i < 9; ++i)
    {
        if (allow[i] == 0) continue;
        s.p[r][c] = allow[i];
        Sudoku v = s.solidSolve();
        if (v.ok() == 1) // Решение найдено
        {
            res = v;
            count++;
            if (count > 1) break;
            continue;
        }
        trySolve(s,res,count);
        if (count > 1) break;
    }
    s.p[r][c] = 0;
    --level;
}


void Sudoku::Generate()
{
    for(int r = 0; r < 9; ++r)
        for(int c = 0; c < 9; ++c)
            p[r][c] = (r == 0) ? c + 1 : 0;
    Gen();
    shuffle();
}

bool Sudoku::Gen()
{
    if (zeros() == 0) return true;
    // Ищем первый свободный
    int r, c;
    for(int x = 0; x < 9; ++x)
        for(int y = 0; y < 9; ++y)
        {
            if (p[x][y]) continue;
            r = x;
            c = y;
            break;
        }

    // Составляем массив допустимых значений
    int allow[9] = { 1,2,3,4,5,6,7,8,9 };
    for(int x = 0; x < 9; ++x)
    {
        if (p[r][x]) allow[p[r][x]-1] = 0;
        if (p[x][c]) allow[p[x][c]-1] = 0;
    }
    for(int x = 0; x < 3; ++x)
        for(int y = 0; y < 3; ++y)
            if (p[(r/3)*3+x][(c/3)*3+y])
                allow[p[(r/3)*3+x][(c/3)*3+y]-1] = 0;

    // Пробуем все варианты поочередно
    for(int k = 8; k > 0; --k)
        swap(allow[k],allow[rand()%k]);
    for(int k = 0; k < 9; ++k)
    {
        if (allow[k] == 0) continue;
        p[r][c] = allow[k];
        if (zeros() == 0)
        {
            if (ok() == 1)
            {
                //printf("---------\n");
                //out();
                //printf("---------\n");
                return true;
            }
        }
        if (Gen()) return true;
    }
    p[r][c] = 0;
    return false;
}

#if 0

int main(int argc, const char * argv[]) /*FOLD00*/
{
    srand(time(0));
    int maxzeros = 0;
    for(;;)
    {
        Sudoku t(false);
        t = t.addZeros(70);
        int z = t.zeros();
        if (maxzeros <= z)
        {
            maxzeros = z;
            t.out();
            printf("Zeros = %d\n",z);
            int count;
            t.solve(count).out();
            printf("Count = %d\n",count);
            printf("---------------------\n");
            fflush(stdout);
        }
        //break;
    }
}
#else

int main(int argc, const char * argv[]) /*FOLD00*/
{

    muTimer mt;
    srand(time(0));
    /*Sudoku s("005300000"
             "800000020"
             "070010500"
             "400005300"
             "010070006"
             "003200080"
             "060500009"
             "004000030"
             "000009700"); */
    // Sudoku s("052009030000020004060000800609100000007280000020400310170000096000030280000000003");
    //Sudoku s("000006054900000001000070309040000076000605413010004005000000132700930648308000597");
    Sudoku s("800000000003600000070090200050007000000045700000100030001000068008500010090000400");
    //Sudoku s("000000000000000000070090200050007000000045700000100030001000068008500010090000400");
    //Sudoku s("100000000"
    //         "000100000"
    //         "000000100"
    //         "000000000"
    //         "000000000"
    //         "000000010"
    //         "000000000"
    //         "000000000"
    //         "000000001");
    int count;
    s = s.solve(count);
    mt.stop();
    s.out();
    printf("Count = %d\n",count);
    printf("Zeros = %d\n",s.zeros());

    printf("%llu mks\n",mt.duration());

    exit(0);
    Sudoku t = s;
    t.Generate();
    t.out();
    printf("Zeros = %d, ok = %d\n",t.zeros(), t.ok());
}
#endif

/*
123 456 789
456 789 123
789 123 456

231 564 897
564 897 231
897 231 564

312 645 978
645 978 312
978 312 645
*/

