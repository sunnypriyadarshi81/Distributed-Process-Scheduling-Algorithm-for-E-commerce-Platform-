# Distributed Process Scheduling Algorithm for E-commerce Platform 

## Project Overview

This project involves designing and implementing a priority-based process scheduling algorithm for a distributed e-commerce platform that handles millions of transactions daily. The system consists of multiple servers, each dedicated to specific transaction types. The algorithm optimizes resource allocation among worker threads by prioritizing requests based on thread priority levels and accommodating varying resource demands for different transaction types.

## System Architecture

- **Multiple Servers**: Each server manages a specific type of transaction, such as payment processing or order handling.
- **Worker Threads**: Each server has a pool of worker threads, characterized by:
  - **Priority Level**: Determines the order in which threads are scheduled for processing.
  - **Resources**: Defines the maximum number of requests a thread can handle concurrently.

## Request Handling

- **Request Queueing**: Incoming requests are initially queued and directed to the appropriate service based on the transaction type.
- **Thread Assignment**: Requests are further queued and assigned to worker threads based on their priority levels.

## Algorithm Design

The scheduling algorithm is designed to:

- **Efficiently Allocate Resources**: Optimize resource allocation among worker threads to ensure efficient request processing.
- **Prioritize Higher-Priority Threads**: Allocate resources to higher-priority threads first. If higher-priority threads are not available, the algorithm falls back to allocating resources to lower-priority threads.
- **Consider Transaction Type**: Adjust resource allocation based on the specific resource demands of different transaction types, such as more resource-intensive payment processing compared to order processing.

## Input Specifications

The input consists of:

1. **Number of Services (`n`)**: Total number of services in the system.
2. **Number of Worker Threads per Service (`m`)**: Number of worker threads available for each service.
3. **Thread Information**: For each worker thread, specify:
   - `priority_level resources`
4. **Request Information**: For each request, specify:
   - `transaction_type resources_required`

## Output Specifications

The output includes:

- **Order of Processed Requests**: The sequence in which requests were processed.
- **Average Waiting Time**: The average time requests spent waiting before being processed.
- **Average Turnaround Time**: The average time taken for each request from arrival to completion.
- **Number of Requests Rejected**: The total number of requests that were rejected due to insufficient resources.

## High Traffic Handling

During periods of high traffic, the output will also include:

- **Number of Requests Delayed**: The number of requests that were delayed due to a lack of available resources.
- **Number of Requests Blocked**: The number of requests that were blocked when all worker threads were occupied.

## Usage

To run the scheduler, compile and execute the program. Input the number of services, worker threads, and then provide the priority and resources for each thread followed by the transaction type and required resources for each request.

The program will output the order in which requests are processed, waiting times, turnaround times, and statistics about rejected, delayed, and blocked requests.
