#include "c_img.h"
#include "seamcarving.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//do we need to include cases where the width is 1??

int min(int one, int two){
    if(one>two){
        return two;
    }
    else{
        return one;
    }
}

int min3(int one, int two, int three){
    if(one>two){
        if(two<three){
            return two;
        } else{
            return three;
        }
    }
    else{
        if(one<three){
            return one;
        } else{
            return three;
        }
    }
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    create_img(grad, im->height, im->width);
    uint8_t leftr = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t rightr = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t leftg = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t rightg = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t leftb = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t rightb = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t topr = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t bottomr = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t topg = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t bottomg = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t topb = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t bottomb = (uint8_t)malloc(sizeof(uint8_t));
    int y;
    for(y=0;y<((im)->height);y++){
        int x;
        for(x=0;x<((im)->width);x++){
            if(x==0){
                leftr = get_pixel(im, y, im->width -1, 0);
                leftg = get_pixel(im, y, im->width -1, 1);
                leftb = get_pixel(im, y, im->width -1, 2);
            } else{
                leftr = get_pixel(im, y, x-1, 0);
                leftg = get_pixel(im, y, x-1, 1);
                leftb = get_pixel(im, y, x-1, 2);
            }
            if(x == (im->width - 1)){
                rightr = get_pixel(im, y, 0, 0);
                rightg = get_pixel(im, y, 0, 1);
                rightb = get_pixel(im, y, 0, 2);

            }else{
                rightr = get_pixel(im, y, x+1, 0);
                rightg = get_pixel(im, y, x+1, 1);
                rightb = get_pixel(im, y, x+1, 2);
            }

            if(y==0){
                topr = get_pixel(im, im->height - 1, x, 0);
                topg = get_pixel(im, im->height - 1, x, 1);
                topb = get_pixel(im, im->height - 1, x, 2);
            } else{
                topr = get_pixel(im, y-1, x, 0);
                topg = get_pixel(im, y-1, x, 1);
                topb = get_pixel(im, y-1, x, 2);
            }
            if(y == (im->height - 1)){
                bottomr = get_pixel(im, 0, x, 0);
                bottomg = get_pixel(im, 0, x, 1);
                bottomb = get_pixel(im, 0, x, 2);
            } else{
                bottomr = get_pixel(im, y+1, x, 0);
                bottomg = get_pixel(im, y+1, x, 1);
                bottomb = get_pixel(im, y+1, x, 2);
            }
            int sum = pow((double)(bottomr - topr) , 2) + pow((double)(bottomg - topg) , 2) + pow((double)(bottomb - topb) , 2);
            int sum2 = pow((double)(leftr - rightr) , 2) + pow((double)(leftg - rightg) , 2) + pow((double)(leftb - rightb) , 2);
            uint8_t toadd = (uint8_t)malloc(sizeof(uint8_t));
            toadd = (uint8_t)(sqrt(sum+sum2)/10);
            set_pixel(*grad, y, x, toadd, toadd, toadd);
        }
    
    }
            


}


void dynamic_seam(struct rgb_img *grad, double **best_arr){
    *best_arr = (double *)malloc(sizeof(double) * grad->width * grad->height);
    int z;
    for(z=0;z<(grad->width);z++){
        (*best_arr)[z] = get_pixel(grad, 0, z, 0);
    }
    int y;
    if(grad->width == 1){
        for(y=1;y<grad->height;y++){
            (*best_arr)[y] = get_pixel(grad, y, 0, 0) + (*best_arr)[(y-1)];
        }
        return;
    }
    
    int x;
    for(y=1;y<(grad->height);y++){
        for(x=0;x<(grad->width);x++){
            if(x==0){
                (*best_arr)[y*(grad->width) + x]  = get_pixel(grad, y, x, 0) + min((*best_arr)[(y-1)*(grad->width) + x], (*best_arr)[(y-1)*(grad->width) + x + 1]);
            } else if(x == (grad->width - 1)){
                (*best_arr)[y*(grad->width) + x]  = get_pixel(grad, y, x, 0) + min((*best_arr)[(y-1)*(grad->width) + x], (*best_arr)[(y-1)*(grad->width) + x - 1]);
            } else{
                (*best_arr)[y*(grad->width) + x]  = get_pixel(grad, y, x, 0) + min3((*best_arr)[(y-1)*(grad->width) + x], (*best_arr)[(y-1)*(grad->width) + x - 1],(*best_arr)[(y-1)*(grad->width) + x + 1] );
            }
        }
    }
}

void recover_path(double *best, int height, int width, int **path){
    *path = (int *)malloc(sizeof(int) * height);
    int min = best[(height-1) * width];
    int loc = 0;
    int x;
    if(width == 1){
        for(x=0;x<height-1;x++){
            (*path)[height] = 0;
        }
    }
    for(x=1;x<width;x++){
        if(min > best[((height-1) * width) + x]){
            min = best[((height-1) * width) + x];
            loc = x;
        }
    }
    (*path)[height - 1] = loc;
    int y;
    int prev;
    for(y=(height-2);y>-1;y--){
        prev = (*path)[y+1];
        if(prev == 0){
            if(best[y*width + prev] > best[y*width + prev + 1]){
                (*path)[y] = prev + 1;
            } else{
                (*path)[y] = prev;
            }
        }
        else if(prev == width - 1){
            if(best[y*width + prev] > best[y*width + prev - 1]){
                (*path)[y] = prev - 1;
            } else{
                (*path)[y] = prev;
            }
        }else{
            if(best[y*width + prev] > best[y*width + prev - 1]){
                if(best[y*width + prev -1 ] > best[y*width + prev + 1]){
                    (*path)[y] = prev + 1;
                } else{
                    (*path)[y] = prev - 1;
                }       
            } else{
                if(best[y*width + prev] > best[y*width + prev + 1]){
                    (*path)[y] = prev + 1;
                } else{
                    (*path)[y] = prev;
                }
            }
        }

    }
}
void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    create_img(dest, src->height, src->width - 1);
    int y;
    int x;
    int changed;
    uint8_t r = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t g = (uint8_t)malloc(sizeof(uint8_t));
    uint8_t b = (uint8_t)malloc(sizeof(uint8_t));
    for(y=0;y<src->height;y++){
        changed = 0;
        for(x=0;x<src->width;x++){
            if(path[y] == x){
                changed = 1;
            }
            else if(changed){
                r = get_pixel(src, y, x, 0);
                g = get_pixel(src, y, x, 1);
                b = get_pixel(src, y, x, 2);
                set_pixel(*dest, y, x-1, r, g, b);
            } else{
                r = get_pixel(src, y, x, 0);
                g = get_pixel(src, y, x, 1);
                b = get_pixel(src, y, x, 2);
                set_pixel(*dest, y, x, r, g, b);
            }
        }

    }
}

// int main(){
//     struct rgb_img *im;
//     struct rgb_img *cur_im;
//     struct rgb_img *grad;
//     double *best;
//     int *path;

//     read_in_img(&im, "HJoceanSmall.bin");
    
//     for(int i = 0; i < 200; i++){
//         printf("i = %d\n", i);
//         calc_energy(im,  &grad);
//         dynamic_seam(grad, &best);
//         recover_path(best, grad->height, grad->width, &path);
//         remove_seam(im, &cur_im, path);

//         char filename[200];
//         sprintf(filename, "img%d.bin", i);
//         write_img(cur_im, filename);


//         destroy_image(im);
//         destroy_image(grad);
//         free(best);
//         free(path);
//         im = cur_im;
//     }
//     destroy_image(im);
// }