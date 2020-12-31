//
// Created by pedro-teixeira on 29/11/2020.
//

int test() {
  return 5;
}

int main(int argc, char *argv[]) {
  int a = 7;
  int b = a * argc;
  b++;
  return b + test();
}