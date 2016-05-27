
#include "sand.h"

uint32_t rand_x = 23;
uint32_t rand_y = 56;
uint32_t rand_z = 89;
uint32_t rand_w = 1112;

uint32_t myRand()
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

int32_t positive(int32_t in)
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

void gravity(int32_t angle)
{
	int32_t probabilities[8];
	int32_t rand_max = 0;
	int32_t i, x, y;
	int32_t offs_x, offs_y;
	int32_t check_x, check_y;
	int32_t activeMatrix;

	//calculate the probability for each direction
	for (i = 0; i < 8; i++)
	{
		probabilities[i] = probability(angle, i * 45);
		rand_max += probabilities[i];
	}

	for (y = 0; y < 16 * MULTIPLY; ++y)
	{
		if(y < 8 * MULTIPLY)
		{
			activeMatrix = 0;
		}
		else
		{
			activeMatrix = 1;
		}
		for (x = 0; x < 8 * MULTIPLY; ++x)
		{
			if (calcMatrix[y][x] == 1) // sand in the field
			{
				int32_t choosen_dir = 0;
				int32_t rand_num = myRand() % rand_max;

				for (i = 0; i < 8; i++)
				{
					rand_num -= probabilities[i];
					if (rand_num < 0)
					{
						choosen_dir = i;
						break;
					}
				}

				offs_x = pseudoCos(choosen_dir - 3);
				offs_y = pseudoCos(choosen_dir - 1);

				for (i = probabilities[choosen_dir] / (MULTIPLY + MULTIPLY - 1); i > 1; i--)
				{
					check_x = (offs_x * i) + x;
					check_y = (offs_y * i) + y;

					if ((check_x >= 0) &&
						(check_x < 8 * MULTIPLY) &&
						(check_y >= (activeMatrix * 8 * MULTIPLY)) &&
						(check_y < (8 * MULTIPLY) * (activeMatrix + 1)))
					{
						if (calcMatrix[check_y][check_x] == 0)
						{
							calcMatrix[y][x] = 0;
							calcMatrix[check_y][check_x] = 1;
							break;
						}
					}
				}
			}
		}
	}
}

int32_t pseudoCos(int32_t dir)
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


int32_t probability(int32_t gravity_angle, int32_t neighbor_angle)
{
	//each val in 0 - 360
	while (gravity_angle < 0)    { gravity_angle += 360;  }
	while (gravity_angle > 360)  { gravity_angle -= 360;  }
	while (neighbor_angle < 0)   { neighbor_angle += 360; }
	while (neighbor_angle > 360) { neighbor_angle -= 360; }


	int32_t ret;

	ret = positive(gravity_angle - neighbor_angle);
	if (ret > 180)
	{
		ret = 360 - ret;
	}

	ret = 100 * (180 - ret) / 180; // ret in 0-100
	return ret;
}

