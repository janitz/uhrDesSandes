#include "sand.h"

//random variables (init values)
uint32_t rand_x = 23;
uint32_t rand_y = 56;
uint32_t rand_z = 89;
uint32_t rand_w = 1112;

int32_t sandCount0 = 0;
int32_t sandCount1 = 0;

//tan array (0-89 deg)
float tangens[90] = {0,      0.0175, 0.0349, 0.0524, 0.0699, 0.0875, 0.1051, 0.1228, 0.1405, 0.1584,
					 0.1763, 0.1944, 0.2126, 0.2309, 0.2493, 0.2679, 0.2867, 0.3057, 0.3249, 0.3443,
					 0.364,  0.3839, 0.404,  0.4245, 0.4452, 0.4663, 0.4877, 0.5095, 0.5317, 0.5543,
					 0.5774, 0.6009, 0.6249, 0.6494, 0.6745, 0.7002, 0.7265, 0.7536, 0.7813, 0.8098,
					 0.8391, 0.8693, 0.9004, 0.9325, 0.9657, 1,      1.0355, 1.0724, 1.1106, 1.1504,
					 1.1918, 1.2349, 1.2799, 1.327,  1.3764, 1.4281, 1.4826, 1.5399, 1.6003, 1.6643,
					 1.7321, 1.804,  1.8807, 1.9626, 2.0503, 2.1445, 2.246,  2.3559, 2.4751, 2.6051,
					 2.7475, 2.9042, 3.0777, 3.2709, 3.4874, 3.7321, 4.0108, 4.3315, 4.7046, 5.1446,
					 5.6713, 6.3138, 7.1154, 8.1443, 9.5144,11.4301,14.3007,19.0811,28.6363,57.29 };



//private functions
int32_t p_probability(int32_t gravity_angle, int32_t neighbor_angle);
uint32_t p_myRand();
int32_t p_positive(int32_t in);
int32_t p_pseudoCos(int32_t dir);
int32_t p_countValues(int32_t value, int32_t xOffset, int32_t yOffset);
int32_t p_min(int32_t in1, int32_t in2);
int32_t p_countBits(int32_t in);
void p_changeValues(int32_t valueFrom, int32_t valueTo, int32_t xOffset, int32_t yOffset, int32_t count);
int32_t p_getRelevantValue(int32_t in);
int32_t p_getBitMask(int32_t width);
void p_fallingSandLine(int32_t angle);
void p_setLineValue(int32_t x, int32_t y);


void gravity(int32_t angle, int32_t randomDir)
{
	int32_t probabilities[8];
	int32_t max_probability;
	int32_t choosen_dir;
	int32_t rand_max, rand_num;
	int32_t i, x, y, yPre;
	int32_t offs_x, offs_y;
	int32_t check_x, check_y;
	int32_t activeMatrix;

	//calculate the probability for each direction to move
	rand_max = 0;
	max_probability = 0;
	choosen_dir = 0;
	for (i = 0; i < 8; i++)
	{
		probabilities[i] = p_probability(angle, i * 45);
		rand_max += probabilities[i];

		//preselect the direction with the most probability
		if (probabilities[i] > max_probability)
		{
			max_probability = probabilities[i];
			choosen_dir = i;
		}
	}

	//go through the matrix
	//rows
	for (yPre = 0; yPre < 16 * MULTIPLY; ++yPre)
	{
		//changes the order of calculating through the matrix depending on the gravity
		//one peace of sand can possibly be moved multiple times depending on the calculating direction (important !!!)
		if (angle < 90 || angle >= 270)
		{
			y = yPre;
		}
		else
		{
			y = (16 * MULTIPLY) - (yPre + 1);
		}

		//don't allow the sand to change matrix
		activeMatrix = (y < 8 * MULTIPLY) ? 0 : 1;

		//columns
		for (x = 0; x < 8 * MULTIPLY; ++x)
		{

			//if there's currently sand in the field
			if (p_getRelevantValue(calcMatrix[y][x]) == 1)
			{

				//chose a random direction if required
				//otherwise choosen direction is the one with the most probability
				if (randomDir)
				{
					rand_num = p_myRand() % rand_max;
					for (i = 0; i < 8; i++)
					{
						rand_num -= probabilities[i];
						if (rand_num <= 0)
						{
							choosen_dir = i;
							break;
						}
					}
				}

				//get a scalable  x and y offset for the choosen direction
				offs_x = p_pseudoCos(choosen_dir - 3);
				offs_y = p_pseudoCos(choosen_dir - 1);

				//probability(0-100) / 33 = 0-3led pixel
				for (i = probabilities[choosen_dir] * MULTIPLY / 33; i > 0; i--)
				{
					//calculate the absolute position in the matrix
					check_x = (offs_x * i) + x;
					check_y = (offs_y * i) + y;

					//don't allow the sand to leave its led matrix
					if ((check_x >= 0) &&
						(check_x < 8 * MULTIPLY) &&
						(check_y >= (activeMatrix * 8 * MULTIPLY)) &&
						(check_y < (8 * MULTIPLY) * (activeMatrix + 1)))
					{

						//if the target position is free
						if (p_getRelevantValue(calcMatrix[check_y][check_x]) == 0)
						{
							//move the sand
							calcMatrix[y][x] &= ~1; //set the last bit to 0 (get the sand)
							calcMatrix[check_y][check_x] |= 1; //set the last bit to 1 (put the sand)
							break;
						}
					}
				}
			}
		}
	}
}

void sandFlow(int32_t angle, float ratio)
{
	//lets the sand flow if possible and necessary

	int32_t sandToMove, moveableSand;

	//negative if in matrix 1 is to much sand
	sandToMove = ((sandCount0 + sandCount1) * ratio) - sandCount0;

	while (angle < 0)   {angle += 360;}
	while (angle >= 360){angle -= 360;}

	//sand can only flow with the gravity
	if ((angle <= 45 || angle >= 315) && sandToMove < 0)
	{
		sandToMove = -sandToMove;

		//how much sand can move depends on how much sand and how much free space is available
		moveableSand = p_min(p_countValues(1, 0, 7), p_countValues(0, 7, 8));

		//also depends on how much sand has to be moved
		sandToMove = p_min(sandToMove, moveableSand);

		//move the sand
		p_changeValues(1, 0, 0, 7, sandToMove); //get the sand
		p_changeValues(0, 1, 7, 8, sandToMove); //put the sand

		p_fallingSandLine(angle);

	}
	else if ((angle <= 225 && angle >= 135) && sandToMove > 0)
	{
		//how much sand can move depends on how much sand and how much free space is available
		moveableSand = p_min(p_countValues(1, 7, 8), p_countValues(0, 0, 7));

		//also depends on how much sand has to be moved
		sandToMove = p_min(sandToMove, moveableSand);

		//move the sand
		p_changeValues(1, 0, 7, 8, sandToMove); //get the sand
		p_changeValues(0, 1, 0, 7, sandToMove);	//put the sand

		p_fallingSandLine(angle);

	}
}

void sandToWS2812(int32_t filter)
{
	//translates the calcMatrix to the WS2812 buffer array
	//filter uses the average sand amount of the last (1 - 16) cycles

	rgb24_t outCol;
	int32_t i, j, x, y;
	int32_t xPos, yPos;
	int32_t fieldValue;
	int32_t lsb;
	int32_t sand_count, sand_count_FIR;

	int32_t filterMask;

	int32_t reversed; //bool
	int32_t ledNr;

	//how much sand is in which led matrix
	sandCount0 = 0;
	sandCount1 = 0;


	filterMask = p_getBitMask(filter);

	for (y = 0; y < 16; ++y)
	{
		//the led matrix is a serpentine, so every second line is reversed
		reversed = y % 2;

		for (x = 0; x < 8; ++x)
		{
			//sand currently in the pixel
			sand_count = 0;

			//sand in the pixel (last 1 - 16 cycles)
			sand_count_FIR = 0;

			//sub matrix of each pixel
			for (j = 0; j < MULTIPLY; ++j)
			{
				for (i = 0; i < MULTIPLY; ++i)
				{
					//absolute position in the matrix
					xPos = (x * MULTIPLY) + i;
					yPos = (y * MULTIPLY) + j;

					//get the value of the field
					fieldValue = calcMatrix[yPos][xPos];

					//negative would be the mask
					if(fieldValue >= 0)
					{
						//last bit
						lsb = fieldValue & 1;

						//add sand of the last cycles
						sand_count_FIR += p_countBits(fieldValue & filterMask);

						//move the history but preserve the lsb (current sand state)
						//& 0xFFFF is for avoiding to change the sign bit while preserving the history of 16 cycles
						calcMatrix[yPos][xPos] = ((fieldValue << 1) + lsb) & 0xFFFF;

						//count the current sand
						if (lsb)
						{
							sand_count ++;
						}
					}
				}
			}

			//add the counted sand to the right matrix statistic
			if( y < 8)
			{
				sandCount0 += sand_count;
			}
			else
			{
				sandCount1 += sand_count;
			}


			//divide to get the average brightness
			int32_t divisor = MULTIPLY * MULTIPLY * filter;
			outCol.r = sandCol.r * sand_count_FIR / divisor;
			outCol.g = sandCol.g * sand_count_FIR / divisor;
			outCol.b = sandCol.b * sand_count_FIR / divisor;

			//calculate the led nr
			ledNr = y * 8;
			if (reversed)
			{
				ledNr += 7 - x;
			}
			else
			{
				ledNr += x;
			}

			//save the color to the array used by the dma
			WS2812_LED_BUF[ledNr + 1] = outCol;
		}


	}
}

int32_t p_probability(int32_t gravity_angle, int32_t neighbor_angle)
{
	/*
	 * calculates the probability for the sand to move to this direction
	 * the probability is big if the both angles point to the same direction
	 */

	//each val in 0 - 360
	while (gravity_angle < 0)    { gravity_angle += 360;  }
	while (gravity_angle > 360)  { gravity_angle -= 360;  }
	while (neighbor_angle < 0)   { neighbor_angle += 360; }
	while (neighbor_angle > 360) { neighbor_angle -= 360; }

	int32_t ret;

	//get the angle between the both directions
	ret = p_positive(gravity_angle - neighbor_angle);

	//the angle between the directions can not be bigger than 180°
	if (ret > 180)
	{
		ret = 360 - ret;
	}

	// ret in 0-100
	ret = 100 * (180 - ret) / 180;
	return ret;
}

uint32_t p_myRand()
{
	//a simple and fast implementation of a random generator

	//xorshift random from https://en.wikipedia.org/wiki/Xorshift
	uint32_t rand_t;
	rand_t = rand_x;
	rand_t ^= rand_t << 11;
	rand_t ^= rand_t >> 8;
	rand_x = rand_y;
	rand_y = rand_z;
	rand_z = rand_w;
	rand_w ^= rand_w >> 19;
	rand_w ^= rand_t;
	return rand_w;
}

int32_t p_positive(int32_t in)
{
	//removes the sign of an int
	if (in < 0)
	{
		return -in;
	}
	else
	{
		return in;
	}
}

int32_t p_pseudoCos(int32_t dir)
{
	/*
	 * returns the rounded cos for 8 directions
	 *
	 * to get the neighbor fields in the matrix
	 *
	 * in:  int dir (dir * PI / 4 = angle(rad))
	 * out: int cos (rounded to -1;0;1)
	 */

	while(dir < 0){dir += 8;}
	while(dir >= 8){dir -= 8;}

	switch (dir) {
		case 0:
		case 1:
		case 7:
			return 1;
			break;
		case 2:
		case 6:
			return 0;
			break;
		case 3:
		case 4:
		case 5:
			return -1;
			break;
		default:
			return 0;
			break;
	}
}

int32_t p_min(int32_t in1, int32_t in2)
{
	//returns the smaller one of the two inputs
	if (in1 < in2)
	{
		return in1;
	}
	else
	{
		return in2;
	}
}

int32_t p_countBits(int32_t in)
{
	//counts the 16 last significant bits in a fast way
	in = (in & 0x5555) + ((in >> 1) & 0x5555); //55 = 01010101
	in = (in & 0x3333) + ((in >> 2) & 0x3333); //33 = 00110011
	in = (in & 0x0F0F) + ((in >> 4) & 0x0F0F); //0F = 00001111
	in = (in & 0x00FF) + ((in >> 8) & 0x00FF);
	return in;
}

int32_t p_countValues(int32_t value, int32_t xOffset, int32_t yOffset)
{
	//counts the occurrence of a value in a pixel submatrix
	int32_t x, y, count;

	count = 0;

	for (y = 0; y < MULTIPLY; ++y)
	{
		for (x = 0; x < MULTIPLY; ++x)
		{
			int32_t xPos = x + (xOffset * MULTIPLY);
			int32_t yPos = y + (yOffset * MULTIPLY);

			if (p_getRelevantValue(calcMatrix[yPos][xPos]) == value)
			{
				count++;
			}
		}
	}

	return count;
}

void p_changeValues(int32_t valueFrom, int32_t valueTo, int32_t xOffset, int32_t yOffset, int32_t count)
{
	//changes values in a pixel submatrix
	int32_t x, y;

	for (y = 0; y < MULTIPLY; ++y)
	{
		for (x = 0; x < MULTIPLY; ++x)
		{

			int32_t xPos = x + (xOffset * MULTIPLY);
			int32_t yPos = y + (yOffset * MULTIPLY);

			if (p_getRelevantValue(calcMatrix[yPos][xPos]) == valueFrom)
			{
				count--;
				if (count < 0)
				{
					return;
				}

				//only set the last bit
				calcMatrix[yPos][xPos] = (calcMatrix[yPos][xPos] & ~1) + valueTo;

			}
		}
	}
	return;
}

int32_t p_getRelevantValue(int32_t in)
{
	//returns -1;0;1
	//if its not -1 then return the last bit
	if (in != -1)
	{
		in = in & 1;
	}

	return in;
}

int32_t p_getBitMask(int32_t width)
{
	//returns a bitmask
	// 3 would return 0b111
	// 5 would return 0b11111

	return (1 << width) - 1;

}

void p_fallingSandLine(int32_t angle)
{
	int32_t preX, preY;
	int32_t x, y;
	int32_t offsX, offsY;
	int32_t multX, multY;

	if(angle <= 44 || angle >= 316)
	{
		offsX = (8 * MULTIPLY) - 1;
		offsY = 8 * MULTIPLY;
		multX = -1;
		multY = 1;

		//angle from 1 to 89
		if (angle >= 316)
		{
			angle -= 360;
		}
		angle += 45;
	}
	else if(angle <= 224 && angle >= 136)
	{
		offsX = 0;
		offsY = (8 * MULTIPLY) - 1;
		multX = 1;
		multY = -1;

		//angle from 1 to 89
		angle -= 135;
	}
	else
	{
		return;
	}


	for (preY = 0; preY < 8 * MULTIPLY; ++preY)
	{
		y = offsY + (preY * multY);
		x = offsX + (tangens[90 - angle] * preY * multX);
		p_setLineValue(x, y);
	}

	for (preX = 0; preX < 8 * MULTIPLY; ++preX)
	{
		x = offsX + (preX * multX);
		y = offsY + (tangens[angle] * preX * multY);
		p_setLineValue(x, y);
	}

}


void p_setLineValue(int32_t x, int32_t y)
{
	if(((y >= 0) && (y < 16 * MULTIPLY)) &&
	   ((x >= 0) && (x < 8 * MULTIPLY)))
	{
		calcMatrix[y][x] |= 2; //second bit
	}
}
