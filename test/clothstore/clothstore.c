#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
typedef struct product {
	int price;
	char name[256];
} product_t;

#define NUM_OF_PROD 2
int list_products(product_t * prod_list)
{
	int i = 0;
	for (i = 0; i < NUM_OF_PROD; i++) {
		printf("%d.%s - $%d\n", i + 1, prod_list[i].name,
		       prod_list[i].price);
	}
	return 0;
}

int get_price(char *prod, product_t * prod_list)
{
	int i = 0;
	for (i = 0; i < NUM_OF_PROD; i++) {
		if (strcasecmp(prod, prod_list[i].name) == 0) {
			return prod_list[i].price;
		}
	}
	return -1;
}

int main()
{
	char option = -1;
	char *prod = malloc(16);
	product_t *prod_list = malloc(NUM_OF_PROD * sizeof(product_t));
	prod_list[0].price = 500;
	strcpy(prod_list[0].name, "Red Jeans");
	prod_list[1].price = 50;
	strcpy(prod_list[1].name, "Black T-shirt");
	while (1) {
		printf("\nPlease select an option:\n");
		printf("List Products(L):\n");
		printf("Buy Products(B):\n");
		printf("Quit(Q):\n");
		scanf(" %c", &option);
		getchar();
		switch (option) {
		case 'L':
		case 'l':
			list_products(prod_list);
			break;
		case 'Q':
		case 'q':

			//free(prod_list);
			exit(0);
		case 'B':
		case 'b':
			printf("\nEnter the product's name you want to buy\n");

			//gets(prod);
			scanf("%[0-9a-zA-Z ]s", prod);
			int price = get_price(prod, prod_list);
			if (price == -1) {
				printf
				    ("We could not find your product in store. Try again\n");
			}

			else {
				printf
				    ("Your chosen product costs $%d. Buy(y/n)?\n",
				     price);
				scanf(" %c", &option);
				switch (option) {
				case 'y':
				case 'Y':
					printf
					    ("Congratulations! Your purchase of $%d is complete"
					     " and posted on your credit card\n",
					     price);
					break;
				}
			}
			break;
		}
	}
	return 0;
}
