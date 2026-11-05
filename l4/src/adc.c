#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <hal/nrf_saadc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>
#include "adc.h"

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};





void printDebugInfo(void)
{
   printk("printing first adc_dt_spec structure contents for all channels\n");
   for(int i = 0;i<3;i++)
   {
   printk("adc_dt_spec structure, channel %d = \n",i);
   printk("Device pointer = %p\n",adc_channels[1].dev);
   printk("Channel Id = %d\n",adc_channels[i].channel_id);
   printk("Voltage reference = %d\n",adc_channels[i].vref_mv);
   printk("Resolution = %d\n",adc_channels[i].resolution);
   printk("Oversampling = %d\n",adc_channels[i].oversampling);
   printk("\n\n");
   }

}

int initializeADC(void)
{

    int err;
	
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!device_is_ready(adc_channels[i].dev)) {
			printk("ADC controller device not ready\n");
			return -1;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			printk("Could not setup channel #%d (%d)\n", i, err);
			return -1;
		}
	}

    return 0;

}

struct Measurement readADCValue(void)
{
	int16_t buf;
    struct Measurement m;
    struct adc_sequence sequence = {
	  .buffer = &buf,

	  .buffer_size = sizeof(buf),
    };

    printk("ADC reading:\n");
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) 
    {
		int err;
        int32_t val_mv;
        // For debug use... 
		/*
        printk("- %s, channel %d: ",
		       adc_channels[i].dev->name,
		       adc_channels[i].channel_id);
        */

		(void)adc_sequence_init_dt(&adc_channels[i], &sequence);

		err = adc_read(adc_channels[i].dev, &sequence);
		if (err < 0) {
			printk("Could not read (%d)\n", err);
			continue;
		} else {
            if(i==0)
            {
                m.x = val_mv;
            }
            else if (i==1)
            {
               m.y = val_mv;
            }
            else if (i==2)
            {
                m.z = val_mv;
            }           
			//printk("%"PRId16, buf);
		}

		
		val_mv = buf;
		err = adc_raw_to_millivolts_dt(&adc_channels[i],&val_mv);
		if (err < 0) 
        {
			printk(" (value in mV not available)\n");
		} 
        else 
        {
			if(i==0)
            {
                m.x = val_mv;
            }
            else if (i==1)
            {
               m.y = val_mv;
            }
            else if (i==2)
            {
                m.z = val_mv;
            }           
            //printk(" = %"PRId32" mV\n", val_mv);
		}
	}

    uint16_t highest_value = 1800;
    uint16_t lowest_value = 1200;

    float center_value = ((float)highest_value + (float)lowest_value) / 2.0f;

    float x_raw_value = m.x - center_value;
    float y_raw_value = m.y - center_value;
    float z_raw_value = m.z - center_value;

    float x_distance = abs((int)x_raw_value);
    float y_distance = abs((int)y_raw_value);
    float z_distance = abs((int)z_raw_value);

    float longest_distance = max(x_distance, max(y_distance, z_distance));

    if (x_distance == longest_distance) {
        if (x_raw_value < 0.0f) {
            m.direction = 0;
        }
        else {
            m.direction = 1;
        }
    }
    else if (y_distance == longest_distance) {
        if (y_raw_value < 0.0f) {
            m.direction = 2;
        }
        else {
            m.direction = 3;
        }
    }
    else {
        if (z_raw_value < 0.0f) {
            m.direction = 4;
        }
        else {
            m.direction = 5;
        }
    }

    return m;
}
