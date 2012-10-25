#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {
    
    hardware.setup();
    hardware.setLedOption(LED_YELLOW);
    depthContext.setup();
    depthValues.setup(&depthContext);
    frameImage.setup(&depthContext);
    //snapshot.setup(&depthContext);
    //color_image.setup(&depthContext);
    //previousDepth = new struct imagePixel[depthValues.getWidth() * depthValues.getHeight()];
    
	ofBackground(0,0, 0);
    cout<<"depth value width and height is  "<<depthValues.getWidth()<< " "<<depthValues.getHeight();
    depthImage.loadImage("./nemo.jpg");
    //color_image.setFromPixels(frameImage.getPixels(), depthImage.getWidth(), depthImage.getHeight(), OF_IMAGE_COLOR);
    //depthImage.setFromPixels(depthValues.getDepthPixels(0, 10000), depthValues.getWidth(),depthImage.getHeight(), OF_IMAGE_COLOR_ALPHA );
    for (int i= 0; i<640; i = i + 1)
    {
        for (int j = 0; j<480; j = j  +1)
        {
            previousColorSum[i][j] = 0;
            previousDepth[i][j].depth = 0;
            previousDepth[i][j].changed = false;
            previousDepth[i][j].initialTime = 0;
            colorDarkenedValues[i][j].darkened = false;
            colorDarkenedValues[i][j].firstChange = true;
            colorDarkenedValues[i][j].secondChange = true;
            colorDarkenedValues[i][j].thirdChange = true;
        }
        
    }
    imageTaken = 0;
    rows = 51;
    column = 62;
    colorThreshold = 8750;
    depthThresholdValue = 42000;
    considerColor = true;
    considerDepth = true;
    factor1 = 0.5;
    factor2 = 0.5;
    
}


//--------------------------------------------------------------
void testApp::update(){
	hardware.update();
    depthContext.update();
    depthValues.update();
    depthImage.update();
    frameImage.update();
    color_image.update();
    snapshot.update();
}

//--------------------------------------------------------------
void testApp::draw_squares(){
    
    depthImage.draw(0,0,depthValues.getWidth(),depthValues.getHeight());
    noiseReduction();
    
    
    for (int i =0; i < depthValues.getWidth(); i++)
    {
        for(int j=0; j<depthValues.getHeight(); j++)
        {
            //cout<<" the value of time outside loop is"<<time(NULL);
            if (time(NULL) - previousDepth[i][j].initialTime>0.5 && previousDepth[i][j].changed == true )
            {
                previousDepth[i][j].initialTime = 0;
                //cout<<" the value of current time is "<<time(NULL);
                //cout<<" the value of previousDepthTime is "<<previousDepth[i][j].initialTime;
                previousDepth[i][j].changed = false;
                depthImage.setColor(i, j, depthValues.getPixelColor(i, j));
            }
            if ((depthValues.getPixelDepth(i, j) - previousDepth[i][j].depth > 1000) || (depthValues.getPixelDepth(i, j) - previousDepth[i][j].depth < -1000))
            {
                depthImage.setColor(i, j, ofColor::red);
                previousDepth[i][j].changed = true;
                previousDepth[i][j].initialTime = time(NULL);
            }
            else
            {
                if (previousDepth[i][j].changed == false)
                {
                    depthImage.setColor(i,j,depthValues.getPixelColor(i, j));
                    //noiseReduction();
                }
                
            }
            
            previousDepth[i][j].depth = depthValues.getPixelDepth(i, j);
        }
    }
    
}
void testApp::noiseReduction()
{
    const int bandSize=25;
    struct noiseDepth innerBand[bandSize];
    //initialize the inner band matrix
    for (int c=0; c<bandSize; c=c+1)
    {
        innerBand[c].depth = 0;
        innerBand[c].frequency = 0;
    }
    
    //see if any of the pixel is 0
    for (int i=0; i<depthValues.getWidth(); i++)
    {
        for (int j=0; j<depthValues.getHeight(); j++)
        {
            if (depthValues.getPixelDepth(i, j) == 0)
            {
                //start storing the nearby pixel depths
                //first do the inner loop
                
                //check the bounds of the inner loop are within the limits of the array
                lowerY = j-2;
                upperY = j+2;
                lowerX = i-2;
                upperX = i+2;
                
                if (i-2 < 0)
                    lowerX=0;
                if (i+2 >639)
                    upperX = 639;
                if (j-2<0)
                    lowerY = 0;
                if (j+2 >479)
                    upperY = 479;
                for (int u = lowerX; u<=upperX; u = u+1)
                {
                    for (int v = lowerY ; v<=upperY; v = v+ 1)
                    {
                        if (depthValues.getPixelDepth(u,v )==0)
                            continue;
                        else 
                        {
                            //check for first non occuring 0 depth value in inner matrix
                            for (int d = 0; d<bandSize; d=d+1)
                            {
                                if (innerBand[d].depth ==0)
                                {
                                    innerBand[d].depth = depthValues.getPixelDepth(u,v);
                                    innerBand[d].frequency = innerBand[d].frequency + 1;
                                }
                                else if (innerBand[d].depth == depthValues.getPixelDepth(u,v))
                                {
                                    innerBand[d].frequency = innerBand[d].frequency + 1;
                                    break;
                                }
                            }
                            
                        }
                    }
                }
                
                // now calculate if the number of non zero depths is greater than the inner matrix threshold
                int innerBandNonZeroDepths = 0;
                for (int e=0; e<bandSize; e= e+1)
                {
                    if (innerBand[e].depth == 0)
                        break;
                    else
                        innerBandNonZeroDepths = innerBandNonZeroDepths  + 1;
                }
                //now check if the number of non zero depths is more than the threshold
                if (innerBandNonZeroDepths > ((bandSize/2)+1))
                {
                    //assign the depth to the selected pixel to be the mode of the depths in the inner band
                    int max=0;
                    int index;
                    for (int g = 0; g<bandSize; g = g+1)
                    {
                        if (innerBand[g].depth !=0)
                        {
                            if(innerBand[g].frequency >max)
                            {
                                max = innerBand[g].frequency;
                                index = g;
                            }
                        }
                        else
                            break;
                    }
                    rectifiedDepthValues[i][j] = innerBand[index].depth;
                    //depthImage.setColor(i, j, innerBand[index].depth);
                }
                else
                {
                    rectifiedDepthValues[i][j] = depthValues.getPixelDepth(i, j);
                    //depthImage.setColor(i, j, depthValues.getPixelDepth(i, j));
                }
            }
            else
                continue;
        }
    }
}
void testApp:: keyPressed(int key)
{
    switch(key)
    {
        case 'c':
                column = column + 1;
                break;
        case 'v':
            column = column - 1;
            break;
        case 'r':
            rows = rows + 1;
            break;
        case 't':
            rows = rows-1;
            break;
        case 'd':
            depthThresholdValue = depthThresholdValue + 1000;
            break;
        case 'f':
            depthThresholdValue = depthThresholdValue - 1000;
            break;
        case 'n':
            colorThreshold = colorThreshold + 1000;
            break;
        case 'm':
            colorThreshold = colorThreshold - 1000;
            break;
        case 'j':
            if (considerColor == true)
                considerColor = false;
            else
                considerColor = true;
            break;
        case 'k':
            if (considerDepth == true)
                considerDepth = false;
            else
                considerDepth = true;
            break;
        case 'i':
            factor1 = factor1 + 0.1;
            break;
        case 'o':
            factor1 = factor1 - 0.1;
            break;
        case 'q':
            factor2 = factor2 + 0.1;
            break;
        case 'w':
            factor2 = factor2 - 0.1;
            break;
            
    }
}
void testApp::draw()
{
    depthImage.draw(0,0,640,400);
    frameImage.draw(depthValues.getWidth(),0,640,400);
    color_image.draw(0,400,640,400);
    noiseReduction();
    //ofDrawBitmapString(msg.str(),900,600);
    
    if (imageTaken ==0)
    {
        cout<<" how many times is this executed";
        color_image.setFromPixels(frameImage.getPixels(), 640, 480, OF_IMAGE_COLOR);
        snapshot.setFromPixels(frameImage.getPixels(), 640, 480, OF_IMAGE_COLOR);
        imageTaken = 1;
        /*
         for (int a =0; a<640; a=a+1)
         {
         for(int d=0; d<480; d=d+1)
         {
         color_image.setColor(a, d, color_image.getColor(a, d)/8);
         colorDarkenedValues[a][d].darkened = true;
         colorDarkenedValues[a][d].darkened_time = time(NULL);
         colorDarkenedValues[a][d].firstChange = false;
         colorDarkenedValues[a][d].secondChange = true;
         colorDarkenedValues[a][d].thirdChange = true;
         }
         }*/
    }
    
    
    
    //int rows = 20 * 4;
    //int column = 16*4;
    int width = depthImage.getWidth()/column;
    int height = depthImage.getHeight()/rows;
    int sum;
    int color_sum;
    //int colorThreshold = 75000/4;
    //int depthThresholdValue = 6000000/60;
    
    
    // calculate the change in depth values for every square
    for (int i=0; i <column; i++)
    {
        for (int j=0; j<rows; j++)
        {
            
            sum = 0;
            color_sum = 0;
            for (int k=i*width; k<(i+1)*width; k++ )
            {
                for (int l = (j*height); l<(j+1)*height; l++)
                {
                    //cout<<" the value of depth is "<<depthValues.getPixelDepth(k, l);
                    //cout<<"the k l pair is "<<k<<" "<<l;
                    if (rectifiedDepthValues[k][l] - previousDepth[k][l].depth > 0)
                        sum = sum + rectifiedDepthValues[k][l] - previousDepth[k][l].depth;
                    else
                        sum = sum + previousDepth[k][l].depth - rectifiedDepthValues[k][l];
                    
                    if ((depthValues.getPixelColor(k, l).r + depthValues.getPixelColor(k, l).g + depthValues.getPixelColor(k, l).b) - previousColorSum[k][l] > 0)
                        color_sum = color_sum + depthValues.getPixelColor(k, l).r + depthValues.getPixelColor(k, l).g + depthValues.getPixelColor(k, l).b  - previousColorSum[k][l];
                    else
                        color_sum = color_sum - depthValues.getPixelColor(k, l).r - depthValues.getPixelColor(k, l).g - depthValues.getPixelColor(k, l).b + previousColorSum[k][l];
                    
                    previousDepth[k][l].depth = rectifiedDepthValues[k][l];
                    previousColorSum[k][l] = depthValues.getPixelColor(k, l).r + depthValues.getPixelColor(k,l).g + depthValues.getPixelColor(k,l).b;
                }
            }
            //cout<<" the value of sum is "<<sum;
            //cout<<" the value of colorsum is "<<color_sum;
            if ((color_sum> colorThreshold && considerColor == true) || (sum>depthThresholdValue && considerDepth == true))
            //if (color_sum > 75000/4)
                //if (sum>6000000/60)
                //if (sum>6000000/144)
            {
                //color that square red for 1 sec
                for (int m = i*width; m<(i+1)*width; m++)
                {
                    for(int n=j*height; n<(j+1)*height; n++)
                    {
                        if (previousDepth[m][n].changed == false)
                            
                        {
                            depthImage.setColor(m, n, ofColor::red);
                            previousDepth[m][n].changed = true;
                            previousDepth[m][n].initialTime = time(NULL);
                            //colorDarkenedValues[m][n].darkened = true;
                            
                            if (colorDarkenedValues[m][n].darkened == false)
                            {
                                //color_image.setColor(m, n, color_image.getColor(m, n)/8);
                                color_image.setColor(m, n, snapshot.getColor(m, n)/8);
                                colorDarkenedValues[m][n].darkened = true;
                                /*
                                colorDarkenedValues[m][n].darkened_time = time(NULL);
                                colorDarkenedValues[m][n].firstChange = false;
                                colorDarkenedValues[m][n].secondChange = true;
                                colorDarkenedValues[m][n].thirdChange = true;
                                 */
                            }
                        }
                        else if (previousDepth[m][n].changed == true && time(NULL) - previousDepth[m][n].initialTime>0.1)
                        {
                            depthImage.setColor(m, n, depthValues.getPixelColor(m, n));
                            previousDepth[m][n].changed = false;
                            
                        }
                        else
                        {
                            depthImage.setColor(m, n, ofColor::red);
                        }
                        
                        
                    }
                }
            }
            else {
                for(int r = i*width; r<(i+1)*width; r++)
                {
                    for (int t = j*height; t<(j+1)*height; t++)
                    {
                        colorDarkenedValues[r][t].darkened = false;
                        if (previousDepth[r][t].changed == true && (time(NULL) - previousDepth[r][t].initialTime < 0.1))
                        {
                            depthImage.setColor(r, t, ofColor::red);
                        }
                        else
                            depthImage.setColor(r, t, depthValues.getPixelColor(r, t));
                    }
                }
            }
        }
    }
    
    
    
    
    //gradually lighten the values of color
    for (int a =0; a<640; a=a+1)
    {
        for (int d = 0; d<480; d=d+1)
        {
            //cout<<"coming in the for loop";
            if (colorDarkenedValues[a][d].darkened == true)
            {
                color_image.setColor(a,d,snapshot.getColor(a,d)/8);
                
                //cout<<" coming here";
                /*
                if ((time(NULL) - colorDarkenedValues[a][d].darkened_time)>1 && colorDarkenedValues[a][d].firstChange == false)
                {
                    //cout<<" in first change";
                    color_image.setColor(a, d, color_image.getColor(a, d) + color_image.getColor(a, d));
                    colorDarkenedValues[a][d].firstChange = true;
                    colorDarkenedValues[a][d].secondChange = false;
                    colorDarkenedValues[a][d].thirdChange = false;
                }
                else if ((time(NULL) - colorDarkenedValues[a][d].darkened_time)>2 && colorDarkenedValues[a][d].secondChange == false)
                {
                    //cout<<" in second change";
                    color_image.setColor(a, d, color_image.getColor(a,d) + color_image.getColor(a, d));
                    colorDarkenedValues[a][d].secondChange = true;
                }
                else if ((time(NULL) - colorDarkenedValues[a][d].darkened_time)>3 && colorDarkenedValues[a][d].thirdChange == false)
                {
                    //cout<<" in third change";
                    color_image.setColor(a,d, color_image.getColor(a,d) + color_image.getColor(a, d));
                    //colorDarkenedValues[a][d].thirdChange = true;
                    colorDarkenedValues[a][d].firstChange = false;
                    colorDarkenedValues[a][d].secondChange = false;
                    colorDarkenedValues[a][d].thirdChange = true;
                    colorDarkenedValues[a][d].darkened = false;
                }*/
                
                
            }
            else
            {
                //color_image.setColor(a, d, snapshot.getColor(a, d)*factor1 + color_image.getColor(a, d )*factor2);
                
                color_image.setColor(a, d, color_image.getColor(a, d) + ( (snapshot.getColor(a, d) - color_image.getColor(a, d))) *factor2);
            }
        }
    }
    stringstream msg;
    msg<<"Number of columns : "<<column << "\t          Press c to increase, v to decrease";
    msg<<"\nNumber of rows : "<<rows<<"\t               Press r to increase and t to decrease ";
    msg<<"\nDepth Threshold is : "<<depthThresholdValue<<"\t        Press d to increase and f to decrease" ;
    msg<<"\nColor Threshold is : "<<colorThreshold<<"\t         Press n to increase and m to decrease";
    msg<<"\nDepth Threshold considered : "<<considerDepth<<"\t     Press k to toggle";
    msg<<"\nColor Threshold considered : "<<considerColor<<"\t     Press j to toggle";
    msg<<"\ni to multiply and o to reduce "<<factor1;
    msg<<"\n q to increase factor 2 and w to reduce factor 2 "<<factor2;
    ofDrawBitmapString(msg.str(), 700, 500);
}