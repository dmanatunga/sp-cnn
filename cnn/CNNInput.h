#ifndef CNN_INPUT_H
#define CNN_INPUT_H

class CNNInput {
  private:
    float* _initialState;
    float* _u;
    int _R;
    int _C;

  public:
    CNNInput(int R, int C);
    ~CNNInput();

    float* initialState() { return _initialState; }
    float* u() { return _u; }
    int R() { return _R; }
    int C() { return _C; }
};

#endif // CNN_INPUT_H
