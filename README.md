# YUVImageProcess
对一个nv21格式的图像读取之后左上角置黑，中心画框，右下角水平镜像和左下角旋转180度的操作

Given a "4160x3120.nv21" yuv image,
1. create 4 threads to run 4 tasks in parall
2. the 4 tasks will be
   suppose the yuv image left and top as (0,0),and right and bottom as（4160，3120）
   task 1, set the left and top corner area as black,from pixle(0，0）to (1440,1080)//图像左上角置黑 
   
   task 2, set a black rect on the center with the rect size 1280x960               //图像中心画黑框，大小1280x960
   
   task 3, flip the right and bottom, the rectangle region is 1440x1080             //图像右下角1440x1080水平镜像 
   
   task 4, rotate 180°for the left and bottom area, the region is 1440x1080        //图像左下角旋转180°
3. save the final result to a new yuv image of 4160x3120_new.nv21

步骤1计5分，步骤2每个taks计3分共12分，步骤3计3分(4个task中确保至少一个正确)

#Standard Input
 4160x3120.nv21 yuv image

#Constraints:
 create 4 threads to excute the task in parall

#Standard Output
 4160x3120_new.nv21 with the above modifications

#Sample
 none


Output
 You can use YUView to double check the crop and scale yuv imag

//////////////////////////
#NOTE:
a. compiling command: gcc thread.c -lpthread
b. running command: ./a.out
//////////////////////
