#include<stdio.h>

int main(void)
{
    int iArray[5][3] = {{1,2,3},{2,4,5},{3,6,9},{4,8,12},{5,10,15}};
    int iSize;
    int iArraySize;
    int iArray_num_elements, iArray_num_rows, iArray_num_cols;

    printf("\n\n");

    iSize = sizeof(int);

    iArraySize = sizeof(iArray);
    printf("Size of 2D int array is : %d\n\n", iArraySize);

    iArray_num_rows = iArraySize / sizeof(iArray[0]);
    printf("\nNo of rows : %d\n\n",iArray_num_rows);

    iArray_num_cols = sizeof(iArray[0]) / iSize;
    printf("\nNo of cols : %d\n\n",iArray_num_cols);

    iArray_num_elements = iArray_num_rows * iArray_num_cols;
    printf("\nNo of elements in 2D array is : %d\n",iArray_num_elements);

    printf("******ROW 1**********\n");
    printf("iArray[0][0] = %d\n", iArray[0][0]);
    printf("iArray[0][1] = %d\n", iArray[0][1]);
    printf("iArray[0][2] = %d\n", iArray[0][2]);

    printf("******ROW 2**********\n");
    printf("iArray[1][0] = %d\n", iArray[1][0]);
    printf("iArray[1][1] = %d\n", iArray[1][1]);
    printf("iArray[1][2] = %d\n", iArray[1][2]);

    printf("******ROW 3**********\n");
    printf("iArray[2][0] = %d\n", iArray[2][0]);
    printf("iArray[2][1] = %d\n", iArray[2][1]);
    printf("iArray[2][2] = %d\n", iArray[2][2]);

    printf("******ROW 1**********\n");
    printf("iArray[3][0] = %d\n", iArray[3][0]);
    printf("iArray[3][1] = %d\n", iArray[3][1]);
    printf("iArray[3][2] = %d\n", iArray[3][2]);

    printf("******ROW 1**********\n");
    printf("iArray[4][0] = %d\n", iArray[4][0]);
    printf("iArray[4][1] = %d\n", iArray[4][1]);
    printf("iArray[4][2] = %d\n", iArray[4][2]);

    return(0);
}
