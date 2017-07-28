#include <stdio.h>
using namespace std;

int main()
{
  int n = 0;

  char c = 'a';

  printf("This is to test getch in a polling loop\n");

  while(1)
  {
    printf("In the loop...\n");

    //while(n < 1000)
    //{
    //   n++;
    //}

    //n = 0;

    c= getch();
    printf("%c\n\n", c);  
  }



}
