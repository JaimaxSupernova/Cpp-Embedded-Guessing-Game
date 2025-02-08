#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#include <conio.h> // for _kbhit() and _getch()
#else
#include <unistd.h>
#endif
#include "rs232.h"
#include <stdbool.h>

void wait_ms(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}
//read function for reading from arduino
int read_serial(int cport_nr, char *buf)
{
    int n = RS232_PollComport(cport_nr, buf, 255);
    buf[n] = '\0'; // Null-terminate the received data
    return n;
}
//write function to write to arduino
int write_serial(int cport_nr, const char *data)
{
    int n = RS232_SendBuf(cport_nr, (unsigned char *)data, strlen(data));
    return n;
}

int main()
{
    //initialising variables
    char guessStr[3];
    int n = 0, guess =0,
        cport_nr = 24,
        bdrate = 9600; /* 9600 baud */
    char mode[] = {'8', 'N', '1', 0},
         buf[256];
    bool valid = false;
    bool set = false;
    bool checked = true;
    int run = 1, scores[50] = {0}, gamecount = 1, failcount =0;
    //check if comport valid
    if (RS232_OpenComport(cport_nr, bdrate, mode, 0))
    {
        printf("Can not open comport\n");
        return 0;
    }
    //print game rules

    printf("you can earn upto 30 points per round\n");
    printf("each wrong answer reduces your points by 1\n");
    printf("you will be hinted when your guess is within 3 of the secret number\n");
    printf("Good luck!\n\n");
    repeatg:
    printf("set guess number on mbed by typing 2 digits and confirm with ""A""!\n");
    set = false;
    while(!set){
        //recieve confirmation from arduino that number has been entered
        n = read_serial(cport_nr, buf);
        if (n > 0)
        {
            printf("Guessing number has been set!\n");
            set = true;
        }
        wait_ms(10);
        }
        while(checked){
        checked = false;
    //get user input for guess
        printf("Please enter a number from 0-30: \n");
        scanf("%d", &guess);
        if(guess>=0&&guess<31){
            valid = true;
            snprintf(guessStr, sizeof(guessStr), "%02d", guess);
            wait_ms(10);
            write_serial(cport_nr, guessStr);
        }
        while(!valid){
                //check entry validity
            if(guess<=0||guess>30){
                printf("Invalid entry, please enter a number from 1-30: ");
                scanf("%d", &guess);
            }
            else{
            valid = true;
            snprintf(guessStr, sizeof(guessStr), "%02d", guess);
            wait_ms(10);
            write_serial(cport_nr, guessStr);
            }
        }
        valid = false;

        while(!checked){
        wait_ms(10);
        n = read_serial(cport_nr, buf);
        printf("\n");
        buf[n] = '\0';
        if (strcmp((char *)buf, "100") == 0)
        {
            printf("Incorrect guess, please try again!\n");
            checked = true;
            failcount++;
        } else if (strcmp((char *)buf, "102") == 0)
        {
            printf("You are within 3 of the number!\n");
            checked = true;
            failcount++;
        } else if(strcmp((char *)buf, "101") == 0)
        {
            char newg[3];
            printf("Game Completed! :)\n");
            scores[gamecount] = 30-failcount;
            if(scores[gamecount] < 0){
            scores[gamecount] = 0;}
            printf("Your score is %d\n", scores[gamecount]);
            gamecount++;
            failcount = 0;
            while(1){
            printf("Would you like to play again? yes or no?\n");
            scanf("%s", newg);
            if(strcmp(newg, "yes")== 0){
                checked = true;
                goto repeatg;

            }
            else if(strcmp(newg, "no")== 0)
            {
                printf("Thanks for playing!\n\nYour scores for each round are:\n");
                printf("Round  | score\n");

                for (int i = 1; i < gamecount; i++)
                {
                    printf(" %d     | %d\n", (i), scores[i]);
                }
                char shut = 'Q';
                wait_ms(10);
                write_serial(cport_nr, &shut);

                n = read_serial(cport_nr, buf);
                buf[n] = '\0';
                exit(0);
            }
            else{
                printf("Invalid entry\n");
            }
            }
            break;
        }
        }

    }
    RS232_CloseComport(cport_nr); // Close the port
    return 0;
}


