#include "coinbase.h"

void
coinbase_init()
{
  char *full_products = NULL;

  if (http_get_request(COINBASE_URL, "/products", &full_products) != 0) {
    pprint_error("unable to get %s%s", __FILE_NAME__, __func__, __LINE__,
        COINBASE_URL, "/products");
    abort();
  }


  __json_value prod_list_json = json_parse(full_products);
  /*
  __json_array prods_json = json_get_array(prod_list_json);
  while (prods_json) {
    __json_value product = prods_json->val;
    __json_object product_data = product->data;
    __json_value product_id = hashmap_get("id", product_data);

    char *prod_cstr = json_get_string(product_id);
    printf("%s\n", prod_cstr);
    prods_json = prods_json->nxt;
  }
  */

  json_free(prod_list_json);
  free(full_products);

}
