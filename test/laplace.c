/* 

 test_vmw.s   -- by Vince Weaver -- 19 February 2000
              -- division code by Asher Lazarus

 Use numerical methods to approximate Laplace's Equation by Averaging
              see page 54 Ramo's "Fields and Waves in Electromagnetics"


          -----5000V-----    One can find approximations for the voltages
         |   .   .   .   |     at the internal points of a infinite-square
         |               |     enclosure, assuming no enclosed charges,
      3000V  .   .   .  6000V  by iteraviely averaging all neighboring
         |               |     points throughout the grid.
         |   .   .   .   |
         |               |   It takes many hundreds of iterations for the
          ----10000V-----      approximation to be good, unfortunately


*/
 

  #define GRIDX 128
  #define GRIDY 128
  #define ITERATIONS 100

  #define GRID_TOP     5000
  #define GRID_LEFT    3000
  #define GRID_RIGHT   6000
  #define GRID_BOTTOM 10000

  short int GRID[GRIDX][GRIDY];

  void fill_grid() {
     int x,y;

     for(x=0;x<GRIDX;x++) for (y=0;y<GRIDY;y++) {
        if (y==0) GRID[x][y]=GRID_TOP;
        else if (x==0) GRID[x][y]=GRID_LEFT;
        else if (x==(GRIDX-1)) GRID[x][y]=GRID_RIGHT;
        else if (y==(GRIDY-1)) GRID[x][y]=GRID_BOTTOM;
        else GRID[x][y]=0;
     }
  }
  
  void dump_array() {
      int x,y;

      for(y=0;y<GRIDY;y++) for (x=0;x<GRIDX;x++) {
         printf("%hd\t",GRID[x][y]);
         if (x==(GRIDX-1)) printf("\n");
      }
  }

  int main() {

     int i,x,y;

     fill_grid();
     dump_array();
     printf("\n");
     for(i=0;i<ITERATIONS;i++) {
        for(x=1;x<GRIDX-1;x++)
  	     for(y=1;y<GRIDY-1;y++)
	        GRID[x][y]=
                  (GRID[x-1][y]+GRID[x+1][y]+GRID[x][y-1]+GRID[x][y+1])/4;
        dump_array();
        printf("\n");
     }
  }

