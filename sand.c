#include "sand.h"

//random variables (init values)
uint32_t rand_x = 23;
uint32_t rand_y = 56;
uint32_t rand_z = 89;
uint32_t rand_w = 1112;

int32_t sandCount0 = 0;
int32_t sandCount1 = 0;


//private functions
int32_t p_probability(int32_t gravity_angle, int32_t neighbor_angle);
uint32_t p_myRand();
int32_t p_positive(int32_t in);
int32_t p_pseudoCos(int32_t dir);
int32_t p_countValues(int32_t value, int32_t xOffset, int32_t yOffset);
int32_t p_min(int32_t in1, int32_t in2);
int32_t p_countBits(int32_t in);
void p_changeValues(int32_t valueFrom, int32_t valueTo, int32_t xOffset, int32_t yOffset, int32_t count);


void gravity(int32_t angle, int32_t randomDir)
{
	int32_t probabilities[8];
	int32_t rand_max = 0;
	int32_t i, x, y;
	int32_t offs_x, offs_y;
	int32_t check_x, check_y;
	int32_t activeMatrix;

	int32_t startVal, compVal, stepVal;

	//calculate the probability for each direction
	for (i = 0; i < 8; i++)
	{
		probabilities[i] = p_probability(angle, i * 45);
		rand_max += probabilities[i];
	}

	if (angle < 90 || angle >= 270)
	{
		startVal = 0;
		compVal = 16 * MULTIPLY;
		stepVal = 1;
	}
	else
	{
		startVal = (16 * MULTIPLY) - 1;
		compVal = -1;
		stepVal = -1;

	}


	for (y = startVal; y != compVal; y += stepVal)
	{
		activeMatrix = (y < 8 * MULTIPLY) ? 0 : 1;

		for (x = 0; x < 8 * MULTIPLY; ++x)
		{
			if ((calcMatrix[y][x] & 1) && (calcMatrix[y][x] != -1)) // sand in the field
			{
				int32_t choosen_dir = 0;
				int32_t rand_num = p_myRand() % rand_max;
				int32_t max_probability = 0;

				for (i = 0; i < 8; i++)
				{
					if (randomDir)
					{
						rand_num -= probabilities[i];
						if (rand_num <= 0)
						{
							choosen_dir = i;
							break;
						}
					}
					else
					{
						if (probabilities[i] > max_probability)
						{
							max_probability = probabilities[i];
							choosen_dir = i;
						}

					}

				}

				offs_x = p_pseudoCos(choosen_dir - 3);
				offs_y = p_pseudoCos(choosen_dir - 1);

				for (i = probabilities[choosen_dir] / (MULTIPLY + MULTIPLY - 1); i > 1; i--)
				{
					check_x = (offs_x * i) + x;
					check_y = (offs_y * i) + y;

					if ((check_x >= 0) &&
						(check_x < 8 * MULTIPLY) &&
						(check_y >= (activeMatrix * 8 * MULTIPLY)) &&
						(check_y < (8 * MULTIPLY) * (activeMatrix + 1)))
					{
						if ((!calcMatrix[check_y][check_x] & 1) && (calcMatrix[check_y][check_x] != -1))
						{
							calcMatrix[y][x] &= ~1;
							calcMatrix[check_y][check_x] |= 1;
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
	int32_t sandToMove, moveableSand;

	//negative if in matrix 1 is to much sand
	sandToMove = ((sandCount0 + sandCount1) * ratio) - sandCount0;

	while (angle < 0)   {angle += 360;}
	while (angle >= 360){angle -= 360;}

	if ((angle <= 45 || angle >= 315) && sandToMove < 0)
	{
		sandToMove = - sandToMove;
		moveableSand = p_min(p_countValues(1, 0, 7), p_countValues(0, 7, 8));
		sandToMove = p_min(sandToMove, moveableSand);
		p_changeValues(0, 1, 7, 8, sandToMove);
		p_changeValues(1, 0, 0, 7, sandToMove);
	}
	else if ((angle <= 225 && angle >= 135) && sandToMove > 0)
	{
		moveableSand = p_min(p_countValues(1, 7, 8), p_countValues(0, 0, 7));
		sandToMove = p_min(sandToMove, moveableSand);
		p_changeValues(0, 1, 0, 7, sandToMove);
		p_changeValues(1, 0, 7, 8, sandToMove);
	}
}

void sandToWS2812(int32_t filterBits)
{
	rgb24_t outCol;
	int32_t i, j, x, y;
	int32_t sand_count, sand_count_FIR;

	int32_t reversed; //bool
	int32_t ledNr;

	sandCount0 = 0;
	sandCount1 = 0;

	int32_t addedBits = 0;
	for(i = 0; i < filterBits; i++)
	{
		addedBits += (1 << i);
	}

	//rows of the led matix
	for (y = 0; y < 16; ++y)
	{
		//the led matrix is a serpentine, so every second line is reversed
		reversed = y % 2;
		//columns  of the led matix

		for (x = 0; x < 8; ++x)
		{
			sand_count = 0;
			sand_count_FIR = 0;

			//each led pixel has a MULTIPLY by MULTIPLY field
			//of corresponding sand parts in the calc matrix
			for (j = 0; j < MULTIPLY; ++j)
			{
				for (i = 0; i < MULTIPLY; ++i)
				{
					int32_t xPos = (x * MULTIPLY) + i;
					int32_t yPos = (y * MULTIPLY) + j;

					if(calcMatrix[yPos][xPos] >= 0)
					{
						sand_count_FIR += p_countBits(calcMatrix[yPos][xPos] & addedBits);
						calcMatrix[yPos][xPos] = ((calcMatrix[yPos][xPos] << 1) + (calcMatrix[yPos][xPos] & 1)) & 0xFFFF;
					}

					//if there's sand in the field
					if ((calcMatrix[yPos][xPos] & 1) && (calcMatrix[yPos][xPos] != -1))
					{
						sand_count ++;
					}
				}
			}

			if( y < 8)
			{
				sandCount0 += sand_count;
			}
			else
			{
				sandCount1 += sand_count;
			}

			//divide to get the average brightness
			outCol.r = sandCol.r * sand_count_FIR / (MULTIPLY * MULTIPLY * filterBits);
			outCol.g = sandCol.g * sand_count_FIR / (MULTIPLY * MULTIPLY * filterBits);
			outCol.b = sandCol.b * sand_count_FIR / (MULTIPLY * MULTIPLY * filterBits);

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
			WS2812_LED_BUF[ledNr] = outCol;
		}


	}
}

int32_t p_probability(int32_t gravity_angle, int32_t neighbor_angle)
{
	//each val in 0 - 360
	while (gravity_angle < 0)    { gravity_angle += 360;  }
	while (gravity_angle > 360)  { gravity_angle -= 360;  }
	while (neighbor_angle < 0)   { neighbor_angle += 360; }
	while (neighbor_angle > 360) { neighbor_angle -= 360; }


	int32_t ret;

	ret = p_positive(gravity_angle - neighbor_angle);
	if (ret > 180)
	{
		ret = 360 - ret;
	}

	ret = 100 * (180 - ret) / 180; // ret in 0-100
	return ret;
}

uint32_t p_myRand()
{
	//xorshift random from https://en.wikipedia.org/wiki/Xorshift
	uint32_t rand_t = rand_x;
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

int32_t p_countValues(int32_t value, int32_t xOffset, int32_t yOffset)
{
	int32_t x, y, count;

	count = 0;

	for (y = 0; y < MULTIPLY; ++y)
	{
		for (x = 0; x < MULTIPLY; ++x)
		{
			int32_t xPos = x + (xOffset * MULTIPLY);
			int32_t yPos = y + (yOffset * MULTIPLY);

			if ((calcMatrix[yPos][xPos] & 1) == value && (calcMatrix[yPos][xPos] != -1))
			{
				count++;
			}
		}
	}

	return count;
}

int32_t p_min(int32_t in1, int32_t in2)
{
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
	//counts the 16 lsb
	in = (in & 0x5555) + ((in >> 1) & 0x5555); //55 = 01010101
	in = (in & 0x3333) + ((in >> 2) & 0x3333); //33 = 00110011
	in = (in & 0x0F0F) + ((in >> 4) & 0x0F0F); //0F = 00001111
	in = (in & 0x00FF) + ((in >> 8) & 0x00FF);
	return in;
}

void p_changeValues(int32_t valueFrom, int32_t valueTo, int32_t xOffset, int32_t yOffset, int32_t count)
{
	int32_t x, y;

	for (y = 0; y < MULTIPLY; ++y)
	{
		for (x = 0; x < MULTIPLY; ++x)
		{

			int32_t xPos = x + (xOffset * MULTIPLY);
			int32_t yPos = y + (yOffset * MULTIPLY);

			if ((calcMatrix[yPos][xPos] & 1) == valueFrom && (calcMatrix[yPos][xPos] != -1))
			{
				calcMatrix[yPos][xPos] &= valueTo;
				calcMatrix[yPos][xPos] |= valueTo;

				count--;
				if (count <= 0)
				{
					return;
				}
			}
		}
	}
	return;
}
