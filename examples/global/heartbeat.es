void main()
{
    uint64_t reports = 0;
    while(true)
    {
        printf("Global heartbeat: %llu\n", ++reports);
        wait(5000);
    }
}
