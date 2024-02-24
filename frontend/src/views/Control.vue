<template>
    <v-container class="container" color="surface" flat>
        <v-row>
            <v-col cols="12">
                <v-card title="Combination" subtitle="Enter your four digit passcode" align="center">
                    <v-card-text>
                        <div class="passcode-container">
                            <v-otp-input length="4" v-model="passcode" class="passcode-input"></v-otp-input>

                        </div>
                    </v-card-text>
                </v-card><br>
                
                <v-btn class="submit" @click="readPasscode();" color="primary">Submit</v-btn>
            </v-col>
        </v-row>

    </v-container>
</template>


<script setup>
/** JAVASCRIPT HERE */

// IMPORTS
import { useMqttStore } from "@/store/mqttStore"; // Import Mqtt Store
import { storeToRefs } from "pinia";
  
import { useAppStore } from "@/store/appStore";
  import {
    ref,
    reactive,
    watch,
    onMounted,
    onBeforeUnmount,
    computed,
  } from "vue";
  import { useRoute, useRouter } from "vue-router";

const passcode = ref([""]);
const Mqtt = useMqttStore(); // Use Mqtt Store
const AppStore = useAppStore(); // Use App Store
 
 
// VARIABLES
const router      = useRouter();
const route       = useRoute();  


// FUNCTIONS
onMounted(()=>{
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    Mqtt.connect(); // Connect to Broker located on the backend
    setTimeout(() => {
      // Subscribe to each topic
      Mqtt.subscribe("620154701");
      Mqtt.subscribe("620154701_sub");
    }, 3000);
});


onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    Mqtt.unsubcribeAll();
});

function readPasscode() {
    // Code to read passcode here
    console.log(passcode.value);
    AppStore.getSetPwd(passcode.value);
    
}

</script>


<style scoped>
/** CSS STYLE HERE */
.container {
    width: 100%;
    background-color: surface;
    align-items: center;
}

.title {
    text-align: center;
}

.passcode-container {
    display: flex;
    justify-content: center;
    
}
.submit {
    display: flex;
    justify-content: center;
    margin: 0 auto;
    align-items: center;
}


.submit {
    
    margin: 0 auto;
    align-items: center;
}

</style>
  