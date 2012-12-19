#pragma OPENCL EXTENSION cl_khr_fp64: enable



__kernel void fmma(float*, float*, float*, long, long);

__kernel void fmma(float* C, float* A, float* B, long nn, long spin)
{
   long fmma_1_3;
   long fmma_2_0_in;
   long fmma_2_0;
   int fmma_0_1;
   bool fmma_0_3;
   int fmma_0_2;
   long fmma_1_0;
   long fmma_1_1;
   long fmma_1_2;
   float fmma_5_0_in;
   float fmma_5_0;
   bool fmma_1_5;
   float fmma_1_6;
   long fmma_5_2;
   float fmma_5_4;
   long fmma_5_5;
   long fmma_5_6;
   float fmma_5_8;
   float fmma_5_9;
   float fmma_5_10;
   long fmma_5_11;
   bool fmma_5_12;
   float fmma_2_1_in;
   float fmma_2_1;
   long fmma_8_2;
   bool fmma_8_3;
   long fmma_4_0;
   int fmma_0_0;
   float fmma_8_0_in;
   float fmma_8_0;
   long fmma_1_7;
   long fmma_5_1_in;
   long fmma_5_1;

   fmma_0_0 = (int)(0x00000000);
   fmma_0_1 = get_global_id((int)(0x00000000));
   fmma_0_2 = get_global_id((int)(0x00000001));
   fmma_0_3 = (spin>(long)(0x0000000000000000));
   if (fmma_0_3)
   {
      fmma_1_0 = convert_long(fmma_0_2);
      fmma_1_1 = (fmma_1_0*nn);
      fmma_1_2 = convert_long(fmma_0_1);
      fmma_1_3 = (fmma_1_1+fmma_1_2);
      fmma_1_5 = (nn>(long)(0x0000000000000000));
      fmma_1_6 = C[fmma_1_3];
      fmma_1_7 = (long)(0x0000000000000000);
      fmma_2_0_in = fmma_1_7;
      fmma_2_1_in = fmma_1_6;
      while(true)
      {
         fmma_2_0 = fmma_2_0_in;
         fmma_2_1 = fmma_2_1_in;
         if (fmma_1_5)
         {
            fmma_4_0 = (long)(0x0000000000000000);
            fmma_5_0_in = fmma_2_1;
            fmma_5_1_in = fmma_4_0;
            while(true)
            {
               fmma_5_0 = fmma_5_0_in;
               fmma_5_1 = fmma_5_1_in;
               fmma_5_2 = (fmma_5_1+fmma_1_1);
               fmma_5_4 = A[fmma_5_2];
               fmma_5_5 = (fmma_5_1*nn);
               fmma_5_6 = (fmma_5_5+fmma_1_2);
               fmma_5_8 = B[fmma_5_6];
               fmma_5_9 = (fmma_5_4*fmma_5_8);
               fmma_5_10 = (fmma_5_0+fmma_5_9);
               fmma_5_11 = (fmma_5_1+(long)(0x0000000000000001));
               fmma_5_12 = (fmma_5_11==nn);
               if (fmma_5_12)
               {
                  break;
               }
               else
               {
                  fmma_5_0_in = fmma_5_10;
                  fmma_5_1_in = fmma_5_11;
                  fmma_5_0_in = fmma_5_10;
                  fmma_5_1_in = fmma_5_11;
               }
            }
            
            fmma_8_0_in = fmma_5_10;
         }
         else
         {
            fmma_8_0_in = fmma_2_1;
            fmma_8_0_in = fmma_2_1;
         }
         fmma_8_0 = fmma_8_0_in;
         C[fmma_1_3] = fmma_8_0;
         fmma_8_2 = (fmma_2_0+(long)(0x0000000000000001));
         fmma_8_3 = (fmma_8_2==spin);
         if (fmma_8_3)
         {
            break;
         }
         else
         {
            fmma_2_0_in = fmma_8_2;
            fmma_2_1_in = fmma_8_0;
            fmma_2_0_in = fmma_8_2;
            fmma_2_1_in = fmma_8_0;
         }
      }
      
      return;
   }
   else
   {
      return;
   }
}
