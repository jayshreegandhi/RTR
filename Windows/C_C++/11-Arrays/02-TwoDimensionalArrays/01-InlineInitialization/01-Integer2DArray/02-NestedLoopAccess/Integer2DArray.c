#include<stdio.h>

int main(void)
{
    int iArray[5][3] = {{1,2,3},{2,4,6},{3,6,9},{4,8,12},{5,10,15}};
    int iSize;
    int iArraySize;
    int iArray_num_elements, iArray_num_rows, iArray_num_cols;
    int i,j;

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

    printf("\nElements in the 2D array:\n");

    for(i =0; i < iArray_num_rows; i++)
    {
        for(j = 0; j < iArray_num_cols; j++)
        {
            printf("iArray[%d][%d] = %d\n",i, j, iArray[i][j]);
        }
        printf("\n\n");
    }
    return(0);
}
