void change_count_two(void)
{
	extern int global_count;
	global_count = global_count + 1;
	printf("Global count = %d\n",global_count);
}
