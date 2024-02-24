<template>
    <v-container  fluid>
      <v-row class="row1">
        <v-col cols="4">
          <v-sheet class="sheet">
            <br />
       
            <v-text-field
              v-model="start"
              label="Start date"
              type="Date"
              dense
              solo-inverted
              class="mr-5"
              :style="{ padding: '17px', maxWidth: '300px' }"
              flat
            ></v-text-field>
            <v-text-field
              v-model="end"
              label="End date"
              type="Date"
              dense
              solo-inverted
              class="mr-5"
              :style="{ padding: '17px', maxWidth: '300px' }"
              flat
            ></v-text-field>
            <br />
            <v-btn
              class="text-none mb-4"
              @click="updateCards();"
              id="analyze"
              color="surface"
              variant="elevated"
            >Analyze</v-btn>
          </v-sheet>
        </v-col>
        <v-col cols="3" align="center">
          <v-card
            title="Average"
            subtitle="For the selected period"
            width="250"
            height="200"
            variant="elevated"
            color="surface"
            density="compact"
            rounded="lg"
            align="center"
          >
            <v-card-item align="center">
              <span class="text-h1 text-black">
                {{ avg.value }}
              </span>
              <small>Gal</small>
            </v-card-item>
          </v-card>
        </v-col>
      </v-row>
      <v-row class="row">
        <v-col cols="12">
          <figure class="highcharts-figure">
            <div id="container"></div>
          </figure>
        </v-col>
        <v-col cols="12">
          <figure class="highcharts-figure">
            <div id="container0"></div>
          </figure>
        </v-col>
      </v-row>
   
    </v-container>
  </template>
  
  <script setup>
  /** JAVASCRIPT HERE */
  
  import Highcharts from "highcharts";
  import more from "highcharts/highcharts-more";
  import Exporting from "highcharts/modules/exporting";
  import { withDirectives } from "vue";
  Exporting(Highcharts);
  more(Highcharts);
  
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
  
  // VARIABLES
  const Mqtt = useMqttStore();
  const AppStore = useAppStore();
  const router = useRouter();
  const route = useRoute();
  const waterMngAnal = ref(null); // Chart object
  const heightWaterLvl = ref(null); // Chart object
  const average= ref(null);
  var start = ref(null);
  var end = ref(null);
  var avg= reactive({ value: 0 });
  // FUNCTIONS
  
  const CreateCharts = async () => {
    // TEMPERATURE CHART
    waterMngAnal.value = Highcharts.chart("container", {
      chart: { zoomType: "x" },
      title: { text: "Water Management Analysis", align: "left" },

      yAxis: {
        title: {
          text: "Water Reserve",
          style: { color: "#000000" },
        },
        labels: { format: "{value} °C" },
      },
  
      tooltip: {
        pointFormat: "Heatindex: {point.x} °C <br/> Temperature: {point.y} °C",
      },
      xAxis: {
        type: "datetime",
        title: { text: "Time", style: { color: "#000000" } },
      },
      tooltip: { shared: true },
      series: [
        {
          name: "Reserve",
          type: "line",
          data: [],
          turboThreshold: 0,
          color: Highcharts.getOptions().colors[0],
        },
      ],
    });
  
    heightWaterLvl.value = Highcharts.chart("container0", {
      chart: { zoomType: "x" },
      title: { text: "Height and Water Level Correlation Analysis", align: "left" },
      yAxis: {
        min: 0,
        title: {
          text: "Height",
          style: { color: "#000000" },
        },
        labels: { format: "{value} %" },
      },
  
      tooltip: {
        pointFormat: "Humidity: {point.x} % ",
      },
      xAxis: {
        type: "scatter",
        title: { text: "Water Height", style: { color: "#000000" } },
      },
      tooltip: { shared: true },
      series: [
        {
          name: "Humidity",
          type: "scatter",
          data: [],
          turboThreshold: 0,
          color: Highcharts.getOptions().colors[0],
        },
      ],
    });
  
 

  };
  
  onMounted(() => {
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    Mqtt.connect(); // Connect to Broker located on the backend
    setTimeout(() => {
      // Subscribe to each topic
      Mqtt.subscribe("620154701");
      Mqtt.subscribe("620154701_sub");
    }, 3000);
    CreateCharts();
  });
  
  onBeforeUnmount(() => {
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    Mqtt.unsubcribeAll();
  });
  
  const updateLineCharts = async () => {
    if (!!start.value && !!end.value) {
      // Convert output from Textfield components to 10 digit timestamps
      let startDate = new Date(start.value).getTime() / 1000;
      let endDate = new Date(end.value).getTime() / 1000;
      // Fetch data from backend
      const data = await AppStore.getAllInRange(startDate, endDate);
      // Create arrays for each plot
      let temperature = [];
      let heatindex = [];
      let humidity = [];
   
      // Iterate through data variable and transform object to format recognized by highcharts
     
      data.forEach((row) => {
        temperature.push({
          x: row.timestamp * 1000,
          y: parseFloat(row.temperature.toFixed(2)),
        });
        heatindex.push({
          x: row.timestamp * 1000,
          y: parseFloat(row.heatindex.toFixed(2)),
        });
        humidity.push({
          x: row.timestamp * 1000,
          y: parseFloat(row.humidity.toFixed(2)),
        });
      });
      // Add data to Temperature and Heat Index chart
      tempHiLine.value.series[0].setData(temperature);
      tempHiLine.value.series[1].setData(heatindex);
      humLine.value.series[0].setData(humidity);
    }
  };
  
  const updateCards = async () => {
    // Retrieve Min, Max, Avg, Spread/Range
    if (!!start.value && !!end.value) {
      // 1. Convert start and end dates collected fron TextFields to 10 digit timestamps
      let startDate = new Date(start.value).getTime() / 1000;
      let endDate = new Date(end.value).getTime() / 1000;
      // 2. Fetch data from backend by calling the API functions
      const average = await AppStore.getCalculateAvg(startDate, endDate);
      console.log(average);
      avg.value = average[0].average.toFixed(1);
      
   
    }
  };
  
  const updateHistogramCharts = async () => {
    // Retrieve Min, Max, Avg, Spread/Range for Column graph
    let startDate = new Date(start.value).getTime() / 1000;
    let endDate = new Date(end.value).getTime() / 1000;
    if (!!start.value && !!end.value) {
      const temp = await AppStore.getFreqDistro( "temperature", startDate, endDate );
      const humid = await AppStore.getFreqDistro("humidity", startDate, endDate);
      const hi = await AppStore.getFreqDistro("heatindex", startDate, endDate);
      // 3. create an empty array for each variable (temperature, humidity and heatindex)
      // see example below
    
      let temperature = [];
      let humidity = [];
      let heatindex = [];
      temp.forEach((row) => {
        temperature.push({ x: row["_id"], y: row["count"] });
      });
      humid.forEach((row) => {
        humidity.push({ x: row["_id"], y: row["count"] });
      });
      hi.forEach((row) => {
        heatindex.push({ x: row["_id"], y: row["count"] });
      });
      
      histo.value.series[0].setData(temperature);
      histo.value.series[1].setData(humidity);
      histo.value.series[2].setData(heatindex);
    }
  };
  
  const updateScatter = async () => {
    if (!!start.value && !!end.value) {
      // Convert output from Textfield components to 10 digit timestamps
      let startDate = new Date(start.value).getTime() / 1000;
      let endDate = new Date(end.value).getTime() / 1000;
      // Fetch data from backend
      const data = await AppStore.getAllInRange(startDate, endDate);
      // Create arrays for each plot
      let scatterPoints1 = [];
      let scatterPoints2 = [];
      // Iterate through data variable and transform object to format recognized by highcharts
      data.forEach((row) => {
        scatterPoints1.push({
          x: parseFloat(row.temperature.toFixed(2)),
          y: parseFloat(row.heatindex.toFixed(2)),
        });
      });
  
      data.forEach((row) => {
        scatterPoints2.push({
          x: parseFloat(row.humidity.toFixed(2)),
          y: parseFloat(row.heatindex.toFixed(2)),
        });
      });
      // Add data to Temperature and Heat Index chart
      tempHiScat.value.series[0].setData(scatterPoints1);
      humScat.value.series[0].setData(scatterPoints2);
    }
  };
  </script>
  
  <style scoped>
  /** CSS STYLE HERE */
  
  .container {
    background-color: #f5f5f5;
    width: 1200px;
  }
  
  .row {
    max-width: 1200px;
  }
  
  .row1 {
    max-width: 1200px;
    padding: 1;
  }
  
  .col1 {
    border: black;
  }
  
  .sheet {
    padding: 2;
    height: 250;
    border-radius: 10px;
    background-color:"primary";
  }
  
  #analyze {
    display: flex;
    justify-content: center;
    margin: 0 auto;
    align-items: center;
}

  Figure {
    border: 2px solid black;
  }
  </style>